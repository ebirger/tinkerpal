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

typedef struct esp8266_t esp8266_t;
struct esp8266_t {
    netif_t netif;
    esp8266_params_t params;
    int state;
    void (*func)(esp8266_t *e);
    const char *func_name;
    /* Socket */
    u32 ip; /* IP in host order */
    u16 port;
    /* Events */
    event_t timeout_evt;
    int timeout_evt_id;
    event_t serial_in_evt;
    /* Match context */
    const char *cur_str;
    const char *cur_str_ptr;
    int cur_str_len;
};

esp8266_t *netif_to_esp8266(netif_t *netif);

/* Set of ugly macros that probably defy all programming best practices.
 * Meant to abstract away the async impl. of serial comm
 */
#define sm_init(e, cb) \
    (e)->func_name = #cb; \
    (e)->func = cb; \
    switch ((e)->state) { case 0:
#define sm_wait(e, timeout) \
    esp8266_timeout_set(e, timeout); \
    (e)->state = __LINE__; \
    return; \
    case __LINE__: \
    esp8266_timeout_del(e)
#define sm_reset(e) (e)->state = 0
#define sm_uninit(e) } sm_reset(e)

/* AT commands related macros */
#define AT_PRINTF(e, fmt, args...) \
    serial_printf((e)->params.serial_port, fmt "\r", args)
#define AT(e, cmd) serial_write((e)->params.serial_port, cmd "\r", sizeof(cmd))
#define MATCH(e, ret) do { \
    esp8266_serial_in_watch_set(e, esp8266_match_trigger); \
    (e)->cur_str = (e)->cur_str_ptr = ret; \
    (e)->cur_str_len = sizeof(ret) - 1; \
    tp_assert((e)->cur_str_len); \
} while(0)
#define AT_MATCH(e, cmd, ret) do { \
    AT(e, cmd); \
    MATCH(e, ret); \
} while(0)

/* Helper functions */
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

static inline void esp8266_timeout_set(esp8266_t *e, int timeout)
{
    e->timeout_evt = (event_t){ .trigger = esp8266_timeout_trigger };
    e->timeout_evt_id = event_timer_set(timeout, &e->timeout_evt);
}

static inline void esp8266_timeout_del(esp8266_t *e)
{
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

static void esp8266_match_trigger(event_t *evt, u32 id, u64 timestamp)
{
    esp8266_t *e = container_of(evt, esp8266_t, serial_in_evt);
    char buf[30];
    int len, i;

    len = esp8266_read(e, buf, sizeof(buf));
    for (i = 0; i < len; i++)
    {
retry:
        if (buf[i] == *e->cur_str_ptr)
            e->cur_str_ptr++;
        else
        {
            if (e->cur_str_ptr == e->cur_str)
                continue;

            /* No match. Start over */
            e->cur_str_ptr = e->cur_str;
            goto retry;
        }
        if (e->cur_str_ptr - e->cur_str == e->cur_str_len)
        {
            /* Full match */
            esp8266_serial_in_watch_del(e);
            e->func(e);
            return;
        }
    }
}

static void esp8266_gen_read_trigger(event_t *evt, u32 id, u64 timestamp)
{
    esp8266_t *e = container_of(evt, esp8266_t, serial_in_evt);
    char buf[30];

    esp8266_read(e, buf, sizeof(buf));
}

/* Actual driver code */
static void esp8266_init(esp8266_t *e)
{
    sm_init(e, esp8266_init);
    serial_enable(e->params.serial_port, 1);
    AT_MATCH(e, "AT+RST", "ready");
    sm_wait(e, 2000);
    tp_out(("%s: reset done.\n", __func__));
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
    /* XXX: must read IP address to validate actual connectivity */
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

static void esp8266_tcp_connect(esp8266_t *e)
{
    u8 *p = (u8 *)&e->ip;

    sm_init(e, esp8266_tcp_connect);
    /* XXX: use common IP serialization function */
    AT_PRINTF(e, "AT+CIPSTART=\"TCP\",\"%u.%u.%u.%u\",%d", p[0], p[1], p[2],
        p[3], e->port);
    MATCH(e, "Linked");
    sm_wait(e, 5000);
    esp8266_serial_in_watch_set(e, esp8266_gen_read_trigger);
    netif_event_trigger(&e->netif, NETIF_EVENT_TCP_CONNECTED);
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

    e->ip = ip;
    e->port = port;
    sm_reset(e);
    esp8266_tcp_connect(e);
    return 0;
}

static int esp8266_netif_tcp_disconnect(netif_t *netif)
{
    esp8266_t *e = netif_to_esp8266(netif);

    sm_reset(e);
    esp8266_tcp_disconnect(e);
    return 0;
}

static const netif_ops_t esp8266_netif_ops = {
    .mac_addr_get = esp8266_netif_mac_addr_get,
    .link_status = NULL,
    .ip_connect = esp8266_netif_ip_connect,
    .ip_disconnect = NULL,
    .tcp_connect = esp8266_netif_tcp_connect,
    .tcp_disconnect = esp8266_netif_tcp_disconnect,
    .ip_addr_get = NULL,
    .free = NULL,
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
    netif_register(&e->netif, &esp8266_netif_ops);
    esp8266_init(e);
    return &e->netif;
}
