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
#ifndef __ETHERIF_H__
#define __ETHERIF_H__

#include "net/netif.h"
#include "util/tp_types.h"

typedef struct etherif_t etherif_t;

typedef struct {
    int (*link_status)(etherif_t *ethif);
    void (*mac_addr_get)(etherif_t *ethif, eth_mac_t *mac);
    int (*packet_recv)(etherif_t *ethif, u8 *buf, int size);
    void (*packet_xmit)(etherif_t *ethif, u8 *buf, int size);
    void (*free)(etherif_t *ethif);
} etherif_ops_t;

struct etherif_t {
    netif_t netif;
    const etherif_ops_t *ops;
    void *ipv4_info;
    void *udp;
    void *dhcpc;
};

void etherif_destruct(etherif_t *ethif);
void etherif_construct(etherif_t *ethif, const etherif_ops_t *ops);

etherif_t *etherif_get_by_id(int id);
etherif_t *netif_to_etherif(netif_t *netif);

static inline int etherif_link_status(etherif_t *ethif)
{
    return ethif->ops->link_status(ethif);
}

static inline void etherif_mac_addr_get(etherif_t *ethif, eth_mac_t *mac)
{
    ethif->ops->mac_addr_get(ethif, mac);
}

static inline int etherif_packet_recv(etherif_t *ethif, u8 *buf, int size)
{
    return ethif->ops->packet_recv(ethif, buf, size);
}

static inline void etherif_packet_xmit(etherif_t *ethif, u8 *buf, int size)
{
    ethif->ops->packet_xmit(ethif, buf, size);
}

static inline void etherif_free(etherif_t *ethif)
{
    ethif->ops->free(ethif);
}

static inline void etherif_ipv4_info_set(etherif_t *ethif, void *ipv4_info)
{
    ethif->ipv4_info = ipv4_info;
    netif_event_trigger(&ethif->netif, NETIF_EVENT_IPV4_CONNECTED);
}

#define etherif_on_port_change_event_set(ethif, ev) \
    netif_on_event_set(&(ethif)->netif, NETIF_EVENT_PORT_CHANGE, ev)
#define etherif_on_port_change_event_clear(ethif) \
    netif_on_event_clear(&(ethif)->netif, NETIF_EVENT_PORT_CHANGE)
#define etherif_port_changed(ethif) \
    netif_event_trigger(&(ethif)->netif, NETIF_EVENT_PORT_CHANGE)

#define etherif_on_packet_received_event_set(ethif, ev) \
    netif_on_event_set(&(ethif)->netif, NETIF_EVENT_PACKET_RECEIVED, ev)
#define etherif_on_packet_received_event_clear(ethif) \
    netif_on_event_clear(&(ethif)->netif, NETIF_EVENT_PACKET_RECEIVED)
#define etherif_packet_received(ethif) \
    netif_event_trigger(&(ethif)->netif, NETIF_EVENT_PACKET_RECEIVED)

#define etherif_on_packet_xmit_event_set(ethif, ev) \
    netif_on_event_set(&(ethif)->netif, NETIF_EVENT_PACKET_XMITTED, ev)
#define etherif_on_packet_xmit_event_clear(ethif) \
    netif_on_event_clear(&(ethif)->netif, NETIF_EVENT_PACKET_XMITTED)
#define etherif_packet_xmitted(ethif) \
    netif_event_trigger(&(ethif)->netif, NETIF_EVENT_PACKET_XMITTED)

#endif
