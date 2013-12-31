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
#include "util/tp_misc.h"
#include "net/packet.h"
#include "net/ether.h"
#include "net/net_debug.h"
#include "mem/tmalloc.h"

const eth_mac_t bcast_mac = { .mac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
const eth_mac_t zero_mac;

static ether_proto_t *protocols;

int ethernet_xmit(etherif_t *ethif, const eth_mac_t *dst_mac, u16 eth_type)
{
    eth_hdr_t *hdr;
    eth_mac_t src_mac;

    etherif_mac_addr_get(ethif, &src_mac);

    /* Prepare Ethernet Header */
    if (!(hdr = packet_push(&g_packet, sizeof(eth_hdr_t))))
	return -1;

    hdr->eth_type = eth_type;
    hdr->dst = *dst_mac;
    hdr->src = src_mac;

    etherif_packet_xmit(ethif, g_packet.ptr, g_packet.length);
    return 0;
}

static void ethernet_packet_received(event_t *e, u32 resource_id)
{
    etherif_t *ethif;
    eth_hdr_t *eth_hdr;
    ether_proto_t *proto;
    u16 eth_type;
    int len;

    ethif = etherif_get_by_id(RES_MAJ(resource_id));

    tp_debug(("Packet received\n"));

    packet_reset(&g_packet, PACKET_RESET_HEAD);
    len = etherif_packet_recv(ethif, g_packet.ptr, g_packet.length);
    if (len < 0)
    {
	tp_err(("Error receiving packet %d\n", len));
	return;
    }

    g_packet.length = len;
    eth_hdr = (eth_hdr_t *)g_packet.ptr;
    eth_type = eth_hdr->eth_type;
    for (proto = protocols; proto && proto->eth_type != eth_type;
	proto = proto->next);
    if (!proto)
    {
	tp_debug(("Unsupported Ethernet Protocol %04x\n", ntohs(eth_type)));
	return;
    }

    packet_pull(&g_packet, sizeof(eth_hdr_t));
    proto->recv(ethif);
}

static event_t ethernet_packet_received_event = {
    .trigger = ethernet_packet_received,
};

void ethernet_unregister_proto(ether_proto_t *proto)
{
    ether_proto_t **iter;

    for (iter = &protocols; *iter && *iter != proto; iter = &(*iter)->next);
    tp_assert(*iter);
    (*iter) = (*iter)->next;
    proto->next = NULL;
}

void ethernet_register_proto(ether_proto_t *proto)
{
    proto->next = protocols;
    protocols = proto;
}

void ethernet_detach_etherif(etherif_t *ethif)
{
    etherif_on_packet_received_event_clear(ethif);
}

void ethernet_attach_etherif(etherif_t *ethif)
{
    etherif_on_packet_received_event_set(ethif,
	&ethernet_packet_received_event);
}
