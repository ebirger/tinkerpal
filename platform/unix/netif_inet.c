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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "util/debug.h"
#include "util/tp_misc.h"
#include "util/tp_types.h"
#include "mem/tmalloc.h"
#include "drivers/resources.h"
#include "drivers/serial/serial.h"
#include "platform/unix/netif_inet.h"
#include "platform/unix/sim.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <linux/in.h> /* XXX: should use netinet/in.h */

typedef struct {
    netif_t netif;
    char dev_name[IFNAMSIZ];
    int socket;
    event_t inet_event;
} netif_inet_t;

#define NET_RES UART_RES(NET_ID)

static netif_inet_t *netif_to_inet(netif_t *netif);

static u32 dev_ip_addr_get(const char *dev_name)
{
    struct ifaddrs *ifaddr, *ifa;
    u32 ret = 0;

    if (getifaddrs(&ifaddr) == -1) 
    {
        perror("getifaddrs");
        return 0;
    }

    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next)
    {
        if (*dev_name && strncmp(ifa->ifa_name, dev_name, IFNAMSIZ -1))
            continue;
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
            continue;

        ret = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
        break;
    }

    freeifaddrs(ifaddr);
    return ret;
}

static void netif_inet_event(event_t *evt, u32 resource_id, u64 timestamp)
{
    netif_inet_t *inet = container_of(evt, netif_inet_t, inet_event);

    netif_event_trigger(&inet->netif, NETIF_EVENT_TCP_DATA_AVAIL);
}

static void netif_inet_mac_addr_get(netif_t *netif, eth_mac_t *mac)
{
    *mac = (eth_mac_t){};
}

static int netif_inet_link_status(netif_t *netif)
{
    return 1;
}

static int netif_inet_ip_connect(netif_t *netif)
{
    netif_event_trigger(netif, NETIF_EVENT_IPV4_CONNECTED);
    return 0;
}

static void netif_inet_ip_disconnect(netif_t *netif)
{
    /* Nothing to do */
}

static int netif_inet_disconnect(netif_t *netif)
{
    netif_inet_t *inet = netif_to_inet(netif);

    if (inet->socket < 0)
        return 0;

    event_watch_del_by_resource(NET_RES);
    unix_sim_remove_fd_event_from_map(NET_ID);
    close(inet->socket);
    inet->socket = -1;
    return 0;
}

static u32 netif_inet_ip_addr_get(netif_t *netif)
{
    return ntohl(dev_ip_addr_get(netif_to_inet(netif)->dev_name));
}

static int netif_inet_connect(netif_t *netif, u8 proto, void *params)
{
    netif_inet_t *inet = netif_to_inet(netif);
    struct sockaddr_in addr;
    tcp_udp_connect_params_t *conn = params;
    long on = 1;
    int rc;

    if (proto != IP_PROTOCOL_TCP)
    {
        tp_err(("Protocol %d not supported\n", proto));
        return -1;
    }

    if (inet->socket > 0)
    {
        tp_err(("netif_inet: only one socket at a time for now\n"));
        return -1;
    }

    inet->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (inet->socket < 0)
    {
        perror("netif_inet: socket");
        return -1;
    }

    unix_sim_add_fd_event_to_map(NET_ID, inet->socket, inet->socket);

    if (setsockopt(inet->socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
    {
        perror("netif_inet: setsockopt");
        goto Error;
    }

    if (*inet->dev_name)
    {
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = dev_ip_addr_get(inet->dev_name);
        addr.sin_port = 0;

        if (bind(inet->socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("netif_inet: bind");
            goto Error;
        }
    }

#if 0
    /* XXX: problem - we only listen on in_fds in unix_select() */
    /* Set the socket to non-blocking */
    flags = fcntl(inet->socket, F_GETFL, 0);
    fcntl(inet->socket, F_SETFL, flags | O_NONBLOCK);
#endif

    inet->inet_event = (event_t){ .trigger = netif_inet_event };
    event_watch_set(NET_RES, &inet->inet_event);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(conn->port);
    addr.sin_addr.s_addr = htonl(conn->ip);
    rc = connect(inet->socket, (struct sockaddr *)&addr, sizeof(addr));
    if (rc < 0 && errno != EINPROGRESS)
    {
        perror("netif_inet: connect");
        goto Error;
    }

    netif_event_trigger(netif, NETIF_EVENT_L4_CONNECTED);
    return 0;

Error:
    netif_inet_disconnect(netif);
    return -1;
}

static int netif_inet_tcp_read(netif_t *netif, char *buf, int size)
{
    netif_inet_t *inet = netif_to_inet(netif);
    int len;

    if (inet->socket < 0)
        return -1;

    len = read(inet->socket, buf, size);
    if (!len)
        netif_event_trigger(netif, NETIF_EVENT_TCP_DISCONNECTED);
    return len;
}

static int netif_inet_tcp_write(netif_t *netif, char *buf, int size)
{
    netif_inet_t *inet = netif_to_inet(netif);

    if (inet->socket < 0)
        return -1;

    return write(inet->socket, buf, size);
}

static void netif_inet_free(netif_t *netif)
{
    netif_inet_t *inet = netif_to_inet(netif);

    netif_unregister(netif);
    tfree(inet);
}

static const netif_ops_t netif_inet_ops = {
    .mac_addr_get = netif_inet_mac_addr_get,
    .link_status = netif_inet_link_status,
    .ip_connect = netif_inet_ip_connect,
    .ip_disconnect = netif_inet_ip_disconnect,
    .connect = netif_inet_connect,
    .tcp_read = netif_inet_tcp_read ,
    .tcp_write = netif_inet_tcp_write,
    .disconnect = netif_inet_disconnect,
    .ip_addr_get = netif_inet_ip_addr_get,
    .free = netif_inet_free,
};

netif_inet_t *netif_to_inet(netif_t *netif)
{
    tp_assert(netif->ops == &netif_inet_ops);
    return (netif_inet_t *)netif;
}

netif_t *netif_inet_new(char *dev_name)
{
    netif_inet_t *inet;

    if (dev_name && strlen(dev_name) >= IFNAMSIZ)
    {
        tp_err(("netif_inet: invalid device name\n"));
        return NULL;
    }

    inet = tmalloc_type(netif_inet_t);
    if (dev_name)
        strcpy(inet->dev_name, dev_name);
    else
        inet->dev_name[0] = '\0';
    inet->socket = -1;

    netif_register(&inet->netif, &netif_inet_ops);
    printf("Created INET Interface\n");
    return &inet->netif;
}
