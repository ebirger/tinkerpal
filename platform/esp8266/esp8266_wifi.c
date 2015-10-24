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
#include "ip_addr.h"
#include "espconn.h"
#include "user_interface.h"
#include "net/netif.h"
#include "net/net_utils.h"
#include "util/debug.h"
#include <string.h>

static netif_t netif;
esp_tcp tcp_sock;
static struct espconn connection;
static int link_on, connection_on;
/* XXX: rx_buf is huge as per SDK documentation recommendation.
 * probably should move to dynamic allocation to save space.
 */
static char rx_buf[3*1500], *rx_head, *rx_tail;

static void wifi_callback(System_Event_t *evt)
{
    switch (evt->event)
    {
    case EVENT_STAMODE_CONNECTED:
        link_on = 1;
        netif_event_trigger(&netif, NETIF_EVENT_READY);
        break;
    case EVENT_STAMODE_DISCONNECTED:
        link_on = 0;
        break;
    case EVENT_STAMODE_GOT_IP:
        netif_event_trigger(&netif, NETIF_EVENT_IPV4_CONNECTED);
        break;
    default:
        break;
    }
}

/* Reading from 'rx_head' and appending to 'rx_tail'.
 * Whenever new data is received, the existing unconsumed data
 * is moved to the start of the buffer (rx_head = rx_buf).
 */
static void data_received(void *arg, char *pdata, unsigned short len)
{
    struct espconn *conn = &connection;

    if (rx_head != rx_buf && rx_head != rx_tail)
    {
        if (rx_head != rx_tail)
            memmove(rx_buf, rx_head, rx_tail - rx_head);
        rx_tail -= rx_head - rx_buf;
        rx_head = rx_buf;
    }

    if (len > sizeof(rx_buf) - (rx_tail - rx_head))
    {
        tp_err("%s: buffer spill! len %d, rx_buf data %d\n", __func__, len,
            rx_tail - rx_head);
        return;
    }

    memcpy(rx_tail, pdata, len);
    rx_tail += len;
    netif_event_trigger(&netif, NETIF_EVENT_TCP_DATA_AVAIL);
    espconn_recv_hold(conn);
}

static void tcp_connected(void *arg)
{
    struct espconn *conn = arg;

    rx_head = rx_tail = rx_buf;
    espconn_regist_recvcb(conn, data_received);
    netif_event_trigger(&netif, NETIF_EVENT_L4_CONNECTED);
}

static void tcp_disconnected(void *arg)
{
    netif_event_trigger(&netif, NETIF_EVENT_TCP_DISCONNECTED);
}

static void esp8266_wifi_mac_addr_get(netif_t *netif, eth_mac_t *mac)
{
    wifi_get_macaddr(0, mac->mac);
}

static int esp8266_wifi_link_status(netif_t *netif)
{
    return link_on;
}

#define SSID "dummy"
#define PSK "psd"

static int esp8266_wifi_ip_connect(netif_t *netif)
{
    static struct station_config config;

    config.bssid_set = 0;
    memcpy(&config.ssid, SSID, sizeof(SSID));
    memcpy(&config.password, PSK, sizeof(PSK));
    wifi_station_set_config(&config);
    wifi_set_event_handler_cb(wifi_callback);
    wifi_station_connect();
    return 0;
}

static void esp8266_wifi_ip_disconnect(netif_t *netif)
{
    wifi_station_disconnect();
}

static int esp8266_wifi_disconnect(netif_t *netif)
{
    static struct espconn *conn = &connection;

    espconn_disconnect(conn);
    connection_on = 0;
    return 0;
}

static int esp8266_wifi_proto_connect(netif_t *netif, u8 proto, void *params)
{
    static struct espconn *conn = &connection;
    tcp_udp_connect_params_t *conn_params = params;

    if (proto != IP_PROTOCOL_TCP)
    {
        tp_err("Protocol %d not supported\n", proto);
        return -1;
    }
    
    if (connection_on)
    {
        tp_err("Already connecting\n");
        return -1;
    }

    tp_out("Connecting... %s:%d\n", ip_addr_serialize(conn_params->ip),
        conn_params->port);
    memset(&connection, 0, sizeof(connection));
    memset(&tcp_sock, 0, sizeof(tcp_sock));

    conn->type = ESPCONN_TCP;
    conn->state = ESPCONN_NONE;
    conn->proto.tcp = &tcp_sock;
    conn->proto.tcp->local_port = espconn_port();
    conn->proto.tcp->remote_port = conn_params->port;
    *(uint32 *)conn->proto.tcp->remote_ip = htonl(conn_params->ip);

    espconn_regist_connectcb(conn, tcp_connected);
    espconn_regist_disconcb(conn, tcp_disconnected);
    connection_on = 1;
    espconn_connect(conn);
    return 0;
}

static u32 esp8266_wifi_ip_addr_get(netif_t *netif)
{
    struct ip_info info;

    wifi_get_ip_info(0, &info);
    return ntohl(info.ip.addr);
}

static int esp8266_wifi_tcp_read(netif_t *netif, char *buf, int size)
{
    static struct espconn *conn = &connection;

    if (size > rx_tail - rx_head)
        size = rx_tail - rx_head;

    if (!size)
        return 0;

    memcpy(buf, rx_head, size);
    rx_head += size;
    if (rx_tail - rx_head) /* More data waiting */
        netif_event_trigger(netif, NETIF_EVENT_TCP_DATA_AVAIL);
    else
        espconn_recv_unhold(conn);

    tp_debug("%s: read %d\n", __func__, size);
    return size;
}

static int esp8266_wifi_tcp_write(netif_t *netif, char *buf, int size)
{
    struct espconn *conn = &connection;

    espconn_sent(conn, (uint8 *)buf, size);
    return size;
}

static void esp8266_wifi_free(netif_t *netif)
{
}

static const netif_ops_t esp8266_wifi_ops = {
    .mac_addr_get = esp8266_wifi_mac_addr_get,
    .link_status = esp8266_wifi_link_status,
    .ip_connect = esp8266_wifi_ip_connect,
    .ip_disconnect = esp8266_wifi_ip_disconnect,
    .proto_connect = esp8266_wifi_proto_connect,
    .tcp_read = esp8266_wifi_tcp_read ,
    .tcp_write = esp8266_wifi_tcp_write,
    .disconnect = esp8266_wifi_disconnect,
    .ip_addr_get = esp8266_wifi_ip_addr_get,
    .free = esp8266_wifi_free,
};

netif_t *esp8266_wifi_new(void)
{
    wifi_set_opmode_current(STATION_MODE);
    netif_register(&netif, "ESP8266 Wi-Fi", &esp8266_wifi_ops);
    return &netif;
}
