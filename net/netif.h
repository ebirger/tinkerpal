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
#ifndef __NETIF_H__
#define __NETIF_H__

#include "util/event.h"
#include "net/net_types.h"
#include "drivers/resources.h"


typedef enum {
    NETIF_EVENT_FIRST = 0,
    NETIF_EVENT_PORT_CHANGE = 0,
    NETIF_EVENT_PACKET_RECEIVED = 1,
    NETIF_EVENT_PACKET_XMITTED = 2,
    NETIF_EVENT_IPV4_CONNECTED = 3,
    NETIF_EVENT_TCP_CONNECTED = 4,
    NETIF_EVENT_COUNT
} netif_event_t;

#define NETIF_RES(netif, event) \
    RES(NETIF_RESOURCE_ID_BASE, (netif)->id, event)

typedef struct netif_t netif_t;

typedef struct {
    void (*mac_addr_get)(netif_t *netif, eth_mac_t *mac);
    int (*link_status)(netif_t *netif);
    int (*ip_connect)(netif_t *netif);
    void (*ip_disconnect)(netif_t *netif);
    /* Addresses in host order */
    int (*tcp_connect)(netif_t *netif, u32 ip, u16 port);
    int (*tcp_disconnect)(netif_t *netif);
    u32 (*ip_addr_get)(netif_t *netif);
    void (*free)(netif_t *netif);
} netif_ops_t;

struct netif_t {
    netif_t *next;
    int id;
    const netif_ops_t *ops;
};

static inline void netif_mac_addr_get(netif_t *netif, eth_mac_t *mac)
{
    netif->ops->mac_addr_get(netif, mac);
}

static inline int netif_link_status(netif_t *netif)
{
    return netif->ops->link_status(netif);
}

static inline int netif_ip_connect(netif_t *netif)
{
    return netif->ops->ip_connect(netif);
}

static inline void netif_ip_disconnect(netif_t *netif)
{
    netif->ops->ip_disconnect(netif);
}

static inline int netif_tcp_connect(netif_t *netif, u32 ip, u16 port)
{
    return netif->ops->tcp_connect(netif, ip, port);
}

static inline int netif_tcp_disconnect(netif_t *netif)
{
    return netif->ops->tcp_disconnect(netif);
}

static inline u32 netif_ip_addr_get(netif_t *netif)
{
    return netif->ops->ip_addr_get(netif);
}

static inline void netif_free(netif_t *netif)
{
    netif->ops->free(netif);
}

static inline void netif_on_event_set(netif_t *netif, netif_event_t event,
    event_t *ev)
{
    event_watch_set(NETIF_RES(netif, event), ev);
}

static inline void netif_on_event_clear(netif_t *netif, netif_event_t event)
{
    event_watch_del_by_resource(NETIF_RES(netif, event));
}

static inline void netif_event_trigger(netif_t *netif, netif_event_t event)
{
    event_watch_trigger(NETIF_RES(netif, event));
}

netif_t *netif_get_by_id(int id);
void netif_register(netif_t *netif, const netif_ops_t *ops);
void netif_unregister(netif_t *netif);

#endif
