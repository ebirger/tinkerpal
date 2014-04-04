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
#include "util/debug.h"
#include "net/arp.h"
#include "net/ipv4.h"
#include "net/ether.h"
#include "net/packet.h"
#include "net/net_debug.h"

#define ARP_HTYPE_ETHERNET 0x1
#define ARP_OPER_REQUEST 0x1
#define ARP_OPER_REPLY 0x2

#define ARP_TIMEOUT (2 * 1000)
#define ARP_RETRIES 4

static ether_proto_t arp_proto;
static arp_resolve_t *pending_resolve;
static int arp_timeout_event_id;
static int arp_retries;

static void arp_timeout(event_t *e, u32 resource_id);

static event_t arp_timeout_event = {
    .trigger = arp_timeout
};

static void arp_pkt_xmit(etherif_t *ethif, u16 oper, const eth_mac_t *tha,
    u32 spa, u32 tpa)
{
    arp_packet_t *arp;
    eth_mac_t sha;

    etherif_mac_addr_get(ethif, &sha);

    /* Prepare ARP header */
    packet_reset(&g_packet, PACKET_RESET_TAIL);
    arp = packet_push(&g_packet, sizeof(arp_packet_t));
    arp->htype = htons(ARP_HTYPE_ETHERNET);
    arp->ptype = htons(ETHER_PROTOCOL_IP);
    arp->hlen = 6;
    arp->plen = 4;
    arp->oper = oper;
    arp->sha = sha;
    arp->spa = spa;
    arp->tha = *tha;
    arp->tpa = tpa;

    arp_timeout_event_id = event_timer_set(ARP_TIMEOUT, &arp_timeout_event);

    ethernet_xmit(ethif, &bcast_mac, htons(ETHER_PROTOCOL_ARP));
}

static void arp_resolve_complete(int status, eth_mac_t mac)
{
    pending_resolve->resolved(pending_resolve, status, mac);
    pending_resolve = NULL;
    event_timer_del(arp_timeout_event_id);
    arp_timeout_event_id = -1;
}

static void arp_resolve_pending(void)
{
    etherif_t *ethif = pending_resolve->ethif;

    arp_pkt_xmit(ethif, htons(ARP_OPER_REQUEST), &zero_mac,
        htonl(ipv4_addr(ethif)), htonl(pending_resolve->ip));
}

static void arp_timeout(event_t *e, u32 resource_id)
{
    if (!pending_resolve)
        return;

    if (arp_retries)
    {
        arp_retries--;
        arp_timeout_event_id = event_timer_set(ARP_TIMEOUT, &arp_timeout_event);
        arp_resolve_pending();
        return;
    }

    tp_err(("ARP request timed out\n"));
    arp_resolve_complete(-1, bcast_mac);
}

static void arp_reply_recv(etherif_t *ethif, arp_packet_t *arp)
{
    if (!pending_resolve || pending_resolve->ethif != ethif)
        return;

    if (arp->spa != htonl(pending_resolve->ip))
        return;

    arp_resolve_complete(0, arp->sha);
}

static void arp_request_recv(etherif_t *ethif, arp_packet_t *arp)
{
    if (ipv4_addr(ethif) != ntohl(arp->tpa))
        return;

    arp_pkt_xmit(ethif, htons(ARP_OPER_REPLY), &arp->sha, arp->tpa, arp->spa);
}

static void arp_recv(etherif_t *ethif)
{
    arp_packet_t *arp = (arp_packet_t *)g_packet.ptr;

    tp_debug(("ARP packet received\n"));

    if (arp->htype != htons(ARP_HTYPE_ETHERNET) ||
        arp->ptype != htons(ETHER_PROTOCOL_IP) ||
        arp->hlen != 6 || arp->plen != 4)
    {
        tp_info(("Only IP Ethernet ARP packets are supported\n"));
        return;
    }

    if (arp->oper == htons(ARP_OPER_REPLY))
        arp_reply_recv(ethif, arp);
    else if (arp->oper == htons(ARP_OPER_REQUEST))
        arp_request_recv(ethif, arp);
    else
        tp_info(("Unsupported ARP oper %d\n", ntohs(arp->oper)));
}

int arp_resolve(arp_resolve_t *resolve)
{
    if (pending_resolve)
    {
        tp_err(("An ARP resolve request is already pending\n"));
        return -1;
    }

    pending_resolve = resolve;
    arp_retries = ARP_RETRIES;
    arp_resolve_pending();
    return 0;
}

void arp_uninit(void)
{
    event_timer_del(arp_timeout_event_id);
    ethernet_unregister_proto(&arp_proto);
}

void arp_init(void)
{
    arp_proto.eth_type = htons(ETHER_PROTOCOL_ARP);
    arp_proto.recv = arp_recv;
    ethernet_register_proto(&arp_proto);
}
