/* Copyright (c) 2013, Eyal Birger
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "drivers/net/esp8266.h"
#include "drivers/serial/serial.h"
#include "util/tp_types.h"
#include "util/tp_misc.h"
#include "util/debug.h"
#include "util/event.h"
#include "mem/tmalloc.h"
#include "net/netif.h"
#include "net/net_utils.h"

typedef struct {
    const char *head;
    const char *ptr;
    int len;
} matched_str_t;

typedef struct esp8266_t esp8266_t;
struct esp8266_t {
    netif_t netif;
    esp8266_params_t params;
    int state;
    void (*func)(esp8266_t *e);
    const char *func_name;
    u32 our_ip;
    /* Socket */
    u32 ip; /* IP in host order */
    u16 port;
    int tcp_connected;
    int data_ready_count_tmp;
    int data_ready_count;
    int write_count;
    char *write_buf;
    /* Events */
    event_t timeout_evt;
    int timeout_evt_id;
    event_t serial_in_evt;
    /* Match context */
#define NUM_MATCHED_STRS 2
    matched_str_t matched_strs[NUM_MATCHED_STRS];
    int match_read_size;
    int matched_idx;
    /* Read line context */
    char line_buf[32];
    int line_buf_count;
    int read_line_status;
};

esp8266_t *netif_to_esp8266(netif_t *netif);

/* Set of ugly macros that probably defy all programming best practices.
 * Meant to abstract away the async impl. of serial comm
 */
#define sm_init(e, cb) \
    (e)->func_name = #cb; \
    (e)->func = cb; \
    switch ((e)->state) { case 0:
#define _sm_wait(e) \
    (e)->state = __LINE__; \
    return; \
    case __LINE__:
#define sm_wait(e, timeout) \
    esp8266_timeout_set(e, timeout, esp8266_timeout_trigger); \
    _sm_wait(e) \
    esp8266_timeout_del(e)
#define sm_sleep(e, time) \
    esp8266_timeout_set(e, time, esp8266_sleep_trigger); \
    _sm_wait(e) \
    esp8266_timeout_del(e)
#define sm_reset(e) (e)->state = 0
#define sm_uninit(e) }

/* AT commands related macros */
#define AT_PRINTF(e, fmt, args...) \
    serial_printf((e)->params.serial_port, fmt "\r", args)
#define AT(e, cmd) serial_write((e)->params.serial_port, cmd "\r", sizeof(cmd))
#define _MATCH(e, match, match_step) do { \
    const char *m = match; \
    esp8266_match(e, &m, 1, match_step); \
} while(0)
#define MATCH(e, match) _MATCH(e, match, 0)
#define AT_MATCH(e, cmd, ret) do { \
    AT(e, cmd); \
    MATCH(e, ret); \
} while(0)

/* Helper functions */
static void esp8266_wait_for_data(esp8266_t *e);

static inline void esp8266_serial_in_watch_set(esp8266_t *e,
    void (*cb)(event_t *evt, u32 id, u64 timestamp))
{
    e->serial_in_evt = (event_t){ .trigger = cb };
    event_watch_set(e->params.serial_port, &e->serial_in_evt);
}

static inline void esp8266_serial_in_watch_del(esp8266_t *e)
{
    event_watch_del_by_resource(e->params.serial_port);
    e->serial_in_evt = (event_t){};
}

static void esp8266_timeout_trigger(event_t *evt, u32 id, u64 timestamp)
{
    esp8266_t *e = container_of(evt, esp8266_t, timeout_evt);

    tp_err(("esp8266: timed out on %s. state %d\n", e->func_name, e->state));
    esp8266_serial_in_watch_del(e);
    sm_reset(e);
}

static void esp8266_sleep_trigger(event_t *evt, u32 id, u64 timestamp)
{
    esp8266_t *e = container_of(evt, esp8266_t, timeout_evt);

    e->func(e);
}

static inline void esp8266_timeout_set(esp8266_t *e, int timeout,
    void (*trigger)(event_t *evt, u32 id, u64 timestamp))
{
    if (!timeout)
    {
        e->timeout_evt_id = -1;
        return;
    }
    e->timeout_evt = (event_t){ .trigger = trigger };
    e->timeout_evt_id = event_timer_set(timeout, &e->timeout_evt);
}

static inline void esp8266_timeout_del(esp8266_t *e)
{
    if (e->timeout_evt_id == -1)
        return;

    event_timer_del(e->timeout_evt_id);
    e->timeout_evt_id = -1;
}

static inline int esp8266_read(esp8266_t *e, char *buf, int size)
{
    int len;

    len = serial_read(e->params.serial_port, buf, size);
    if (e->params.echo_on)
        tp_out_bin(buf, len);
    return len;
}

static void matched_str_init(matched_str_t *m, const char *s)
{
    m->head = m->ptr = s;
    m->len = strlen(s);
}

static int matched_str_match(matched_str_t *m, char c)
{
    if (!m->len)
        return 0;

again:
    if (c == *m->ptr)
        m->ptr++;
    else
    {
        if (m->ptr == m->head)
            return 0;

        /* No match. Start over */
        m->ptr = m->head;
        goto again;
    }

    if (m->ptr - m->head == m->len)
    {
        /* found */
        return 1;
    }

    return 0;
}

static void esp8266_match_trigger(event_t *evt, u32 id, u64 timestamp)
{
    esp8266_t *e = container_of(evt, esp8266_t, serial_in_evt);
    char buf[30];
    int len, i, matched_idx;

    len = sizeof(buf);
    if (e->match_read_size && e->match_read_size < len)
        len = e->match_read_size;
    len = esp8266_read(e, buf, len);
    for (i = 0; i < len; i++)
    {
        for (matched_idx = 0; matched_idx < NUM_MATCHED_STRS; matched_idx++)
        {
            if (!matched_str_match(&e->matched_strs[matched_idx], buf[i]))
                continue;

            /* Full match */
            esp8266_serial_in_watch_del(e);
            e->matched_idx = matched_idx;
            e->func(e);
            return;
        }
    }
}

static void esp8266_match(esp8266_t *e, const char *matches[],
    int num_matches, int match_step_size)
{
    int i;

    tp_assert(num_matches <= NUM_MATCHED_STRS);

    esp8266_serial_in_watch_set(e, esp8266_match_trigger);
    e->matched_idx = -1;
    for (i = 0; i < NUM_MATCHED_STRS; i++)
    {
        const char *s = i < num_matches ? matches[i] : "";

        matched_str_init(&e->matched_strs[i], s);
    }
    e->match_read_size = match_step_size;
}

static void esp8266_gen_trigger(event_t *evt, u32 id, u64 timestamp)
{
    esp8266_t *e = container_of(evt, esp8266_t, serial_in_evt);

    e->func(e);
}

static void esp8266_read_line_trigger(event_t *evt, u32 id, u64 timestamp)
{
    esp8266_t *e = container_of(evt, esp8266_t, serial_in_evt);
    int remaining = sizeof(e->line_buf) - e->line_buf_count;

    if (!remaining)
    {
        tp_err(("esp8266: read line buffer full\n"));
        e->read_line_status = -1;
        e->func(e);
        return;
    }
    e->line_buf_count += esp8266_read(e, e->line_buf + e->line_buf_count,
        remaining);
    if (!strstr(e->line_buf, "\r\n"))
        return; /* Incomplete */

    e->read_line_status = 0;
    esp8266_serial_in_watch_del(e);
    e->func(e);
}

static void esp8266_get_line(esp8266_t *e)
{
    memset(e->line_buf, 0, sizeof(e->line_buf));
    e->line_buf_count = 0;
    esp8266_serial_in_watch_set(e, esp8266_read_line_trigger);
}

/* Actual driver code */
static void esp8266_init(esp8266_t *e)
{
    sm_init(e, esp8266_init);
    serial_enable(e->params.serial_port, 1);
    AT_MATCH(e, "AT+RST", "ready");
    sm_wait(e, 2000);
    tp_out(("%s: reset done.\n", __func__));
    sm_sleep(e, 2000);
    netif_event_trigger(&e->netif, NETIF_EVENT_READY);
    sm_uninit(e);
}

#define SSID "dummy"
#define PSK "psk"

static void esp8266_connect(esp8266_t *e)
{
    sm_init(e, esp8266_connect);
    AT_MATCH(e, "AT+CWMODE=1", "no change");
    sm_wait(e, 100);
    /* TODO: receive connection params */
    AT_PRINTF(e, "AT+CWJAP=\"%s\",\"%s\"", SSID, PSK);
    MATCH(e, "OK");
    sm_wait(e, 2000);
    AT_MATCH(e, "AT+CIPMUX=0", "OK");
    sm_wait(e, 100);
    AT(e, "AT+CIFSR");
    _MATCH(e, "AT+CIFSR\r\r\n", 1);
    sm_wait(e, 100);
    /* Get IP address */
    esp8266_get_line(e);
    sm_wait(e, 100);
    if (e->read_line_status || strstr(e->line_buf, "ERROR"))
    {
        tp_err(("esp8266: failed to aquire IP address\n"));
        return;
    }
    tp_debug(("Got IP [%d] %s", e->line_buf_count, e->line_buf));
    e->our_ip = ip_addr_parse(e->line_buf, e->line_buf_count - 2);
    sm_sleep(e, 5000);
    netif_event_trigger(&e->netif, NETIF_EVENT_IPV4_CONNECTED);
    sm_uninit(e);
}

static void esp8266_netif_mac_addr_get(netif_t *netif, eth_mac_t *mac)
{
    *mac = (eth_mac_t){};
}

static int esp8266_netif_ip_connect(netif_t *netif)
{
    esp8266_t *e = netif_to_esp8266(netif);

    sm_reset(e);
    esp8266_connect(e);
    return 0;
}

static void _esp8266_wait_for_data(esp8266_t *e)
{
    sm_init(e, _esp8266_wait_for_data);
    esp8266_match(e, (const char *[]){"+IPD,", "Unlink"}, 2, 1);
    sm_wait(e, 0);
    if (e->matched_idx == 1)
    {
        e->tcp_connected = 0;
        netif_event_trigger(&e->netif, NETIF_EVENT_TCP_DISCONNECTED);
        esp8266_wait_for_data(e);
        return;
    }

    e->data_ready_count = e->data_ready_count_tmp = 0;
    esp8266_serial_in_watch_set(e, esp8266_gen_trigger);
    while(1)
    {
        char c;

        sm_wait(e, 0);
        if (e->data_ready_count)
        {
            netif_event_trigger(&e->netif, NETIF_EVENT_TCP_DATA_AVAIL);
            continue;
        }

        /* Parse available data count */
        if ((esp8266_read(e, &c, 1)) != 1)
            continue;

        if (c == ':')
        {
            e->data_ready_count = e->data_ready_count_tmp;
            continue;
        }

        if (c < '0' || c > '9')
        {
            tp_err(("esp8266: malformed IPD header\n"));
            esp8266_wait_for_data(e);
            return;
        }

        e->data_ready_count_tmp = e->data_ready_count_tmp * 10 + c - '0';
    }
    sm_uninit(e);
}

static void esp8266_wait_for_data(esp8266_t *e)
{
    sm_reset(e);
    _esp8266_wait_for_data(e);
}

static void esp8266_tcp_connect(esp8266_t *e)
{
    sm_init(e, esp8266_tcp_connect);
    AT_PRINTF(e, "AT+CIPSTART=\"TCP\",\"%s\",%d", ip_addr_serialize(e->ip),
        e->port);
    MATCH(e, "Linked");
    sm_wait(e, 5000);
    e->tcp_connected = 1;
    netif_event_trigger(&e->netif, NETIF_EVENT_TCP_CONNECTED);
    esp8266_wait_for_data(e);
    sm_uninit(e);
}

static void esp8266_tcp_disconnect(esp8266_t *e)
{
    sm_init(e, esp8266_tcp_disconnect);
    AT_MATCH(e, "AT+CIPCLOSE", "OK");
    sm_wait(e, 5000);
    tp_out(("Disconnected\n"));
    sm_uninit(e);
}

static int esp8266_netif_tcp_connect(netif_t *netif, u32 ip, u16 port)
{
    esp8266_t *e = netif_to_esp8266(netif);

    if (e->tcp_connected)
    {
        tp_err(("esp8266: TCP already connected\n"));
        return -1;
    }

    e->ip = ip;
    e->port = port;
    sm_reset(e);
    esp8266_tcp_connect(e);
    return 0;
}

static int esp8266_netif_tcp_read(netif_t *netif, char *buf, int size)
{
    esp8266_t *e = netif_to_esp8266(netif);
    int len;

    if (!e->tcp_connected)
    {
        tp_err(("esp8266 read: TCP not connected\n"));
        return -1;
    }

    if (!e->data_ready_count)
        return 0;

    len = MIN(size, e->data_ready_count);
    len = esp8266_read(e, buf, len);
    e->data_ready_count -= len;
    if (!e->data_ready_count)
        esp8266_wait_for_data(e);
    return len;
}

static void esp8266_tcp_write(esp8266_t *e)
{
    sm_init(e, esp8266_tcp_write);
    AT_PRINTF(e, "AT+CIPSEND=%d", e->write_count);
    MATCH(e, ">");
    sm_wait(e, 1000);
    serial_write(e->params.serial_port, e->write_buf, e->write_count);
    esp8266_wait_for_data(e);
    sm_uninit(e);
}

static int esp8266_netif_tcp_write(netif_t *netif, char *buf, int size)
{
    esp8266_t *e = netif_to_esp8266(netif);

    if (!e->tcp_connected)
    {
        tp_err(("esp8266 write: TCP not connected\n"));
        return -1;
    }
    e->write_buf = buf;
    e->write_count = size;
    sm_reset(e);
    esp8266_tcp_write(e);
    return 0;
}

static int esp8266_netif_tcp_disconnect(netif_t *netif)
{
    esp8266_t *e = netif_to_esp8266(netif);

    if (!e->tcp_connected)
    {
        /* TCP already disconnected. Nothing more to do */
        return 0;
    }

    sm_reset(e);
    esp8266_tcp_disconnect(e);
    return 0;
}

static u32 esp8266_netif_ip_addr_get(netif_t *netif)
{
    esp8266_t *e = netif_to_esp8266(netif);

    return e->our_ip;
}

static void esp8266_netif_free(netif_t *netif)
{
    esp8266_t *e = netif_to_esp8266(netif);

    netif_unregister(netif);
    tfree(e);
}

static const netif_ops_t esp8266_netif_ops = {
    .mac_addr_get = esp8266_netif_mac_addr_get,
    .link_status = NULL,
    .ip_connect = esp8266_netif_ip_connect,
    .ip_disconnect = NULL,
    .tcp_connect = esp8266_netif_tcp_connect,
    .tcp_read = esp8266_netif_tcp_read,
    .tcp_write = esp8266_netif_tcp_write,
    .tcp_disconnect = esp8266_netif_tcp_disconnect,
    .ip_addr_get = esp8266_netif_ip_addr_get,
    .free = esp8266_netif_free,
};

esp8266_t *netif_to_esp8266(netif_t *netif)
{
    tp_assert(netif->ops == &esp8266_netif_ops);
    return (esp8266_t *)netif;
}

netif_t *esp8266_new(const esp8266_params_t *params)
{
    esp8266_t *e = tmalloc_type(esp8266_t);

    e->params = *params;
    sm_reset(e);
    e->tcp_connected = 0;
    netif_register(&e->netif, &esp8266_netif_ops);
    esp8266_init(e);
    return &e->netif;
}
