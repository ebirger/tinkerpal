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
#include <string.h> /* memcpy */
#include "util/debug.h"
#include "net/arp.h"
#include "net/ether.h"
#include "net/packet.h"
#include "net/net_debug.h"

#define ARP_HTYPE_ETHERNET 0x1
#define ARP_OPER_REQUEST 0x1
#define ARP_OPER_REPLY 0x2

static ether_proto_t arp_proto;
static arp_resolve_t *pending_resolve;
static eth_mac_t bcast_mac = { .mac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };

static void arp_pkt_xmit(etherif_t *ethif, u16 oper, eth_mac_t *sha, u8 spa[],
    u8 tpa[])
{
    arp_packet_t *arp;
    eth_hdr_t *eth_hdr;

    packet_reset(&g_packet, PACKET_RESET_TAIL);
    arp = packet_push(&g_packet, sizeof(arp_packet_t));
    arp->htype = htons(ARP_HTYPE_ETHERNET);
    arp->ptype = htons(ETHER_PROTOCOL_IP);
    arp->hlen = 6;
    arp->plen = 4;
    arp->oper = oper;
    arp->sha = *sha;
    memcpy(arp->spa, spa, 4);
    memcpy(arp->tpa, tpa, 4);
    eth_hdr = packet_push(&g_packet, sizeof(eth_hdr_t));
    eth_hdr->eth_type = htons(ETHER_PROTOCOL_ARP);
    eth_hdr->dst = bcast_mac;
    eth_hdr->src = *sha;
    etherif_packet_xmit(ethif, g_packet.ptr, g_packet.length);
}

static void arp_reply_recv(etherif_t *ethif, arp_packet_t *arp)
{
    u32 ip;

    if (!pending_resolve || pending_resolve->ethif != ethif)
	return;

    ip = htonl(pending_resolve->ip);
    if (memcmp((void *)&ip, arp->spa, 4))
	return;

    pending_resolve->resolved(pending_resolve, 0, arp->sha);
    pending_resolve = NULL;
}

static void arp_request_recv(etherif_t *ethif, arp_packet_t *arp)
{
    u32 ip;
    eth_mac_t mac;

    ip = htonl(ethif->ip);
    if (memcmp((void *)&ip, arp->tpa, 4))
	return;

    etherif_mac_addr_get(ethif, &mac);
    arp_pkt_xmit(ethif, htons(ARP_OPER_REPLY), &mac, (u8 *)&ip, arp->spa);
}

static void arp_recv(etherif_t *ethif)
{
    arp_packet_t *arp = (arp_packet_t *)g_packet.ptr;

    tp_out(("ARP packet received\n"));

    arp_packet_dump(arp);

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
    etherif_t *ethif;
    eth_mac_t mac;
    u32 sip, tip;

    if (pending_resolve)
    {
	tp_err(("An ARP resolve request is already pending\n"));
	return -1;
    }

    pending_resolve = resolve;
    ethif = resolve->ethif;

    etherif_mac_addr_get(ethif, &mac);

    sip = htonl(ethif->ip);
    tip = htonl(resolve->ip);
    arp_pkt_xmit(ethif, htons(ARP_OPER_REQUEST), &mac, (u8 *)&sip, (u8 *)&tip);
    return 0;
}

void arp_uninit(void)
{
    ethernet_unregister_proto(&arp_proto);
}

void arp_init(void)
{
    arp_proto.eth_type = htons(ETHER_PROTOCOL_ARP);
    arp_proto.recv = arp_recv;
    ethernet_register_proto(&arp_proto);
}
