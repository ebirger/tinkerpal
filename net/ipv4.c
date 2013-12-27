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
#include "net/ipv4.h"
#include "net/ether.h"
#include "net/packet.h"
#include "net/net_debug.h"
#include "net/net_types.h"

static ether_proto_t ipv4_proto;
static ipv4_proto_t *ipv4_protocols;

static u16 ipv4_hdr_checksum(ip_hdr_t *iph)
{
    u16 *addr = (u16 *)iph, count;
    u32 sum = 0;

    for (count = 20; count; count -= 2)
        sum += *addr++;

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return (u16)~sum;
}

int ipv4_xmit(etherif_t *ethif, const eth_mac_t *dst_mac, u8 protocol,
    u32 src_addr, u32 dst_addr, u16 payload_len)
{
    ip_hdr_t *iph;
    u16 tot_len;

    src_addr = htonl(src_addr);
    dst_addr = htonl(dst_addr);
    tot_len = sizeof(ip_hdr_t) + payload_len;

    /* IPv4 Header */
    if (!(iph = packet_push(&g_packet, sizeof(ip_hdr_t))))
	return -1;

    iph->ver = 4;
    iph->ihl = 5; /* No support for options */
    iph->dscp = 0;
    iph->tot_len = htons(tot_len);
    iph->id = 0; /* Per RFC 6864 - no frags -> field is meaningless */
    iph->frag_off = 0; /* No frags */
    iph->ttl = 255;
    iph->protocol = protocol;
    iph->checksum = 0;
    memcpy(iph->src_addr, (u8 *)&src_addr, 4);
    memcpy(iph->dst_addr, (u8 *)&dst_addr, 4);
    iph->checksum = ipv4_hdr_checksum(iph);

    return ethernet_xmit(ethif, dst_mac, htons(ETHER_PROTOCOL_IP));
}

static void ipv4_recv(etherif_t *ethif)
{
    ip_hdr_t *iph = (ip_hdr_t *)g_packet.ptr;
    ipv4_proto_t *proto;

    tp_debug(("IPv4 packet received\n"));

    for (proto = ipv4_protocols; proto && proto->protocol != iph->protocol;
	proto = proto->next);
    if (!proto)
    {
	tp_debug(("Unsupported IPv4 Protocol %02x\n", iph->protocol));
	return;
    }

    packet_pull(&g_packet, sizeof(ip_hdr_t));
    proto->recv(ethif);
}

void ipv4_unregister_proto(ipv4_proto_t *proto)
{
    ipv4_proto_t **iter;

    for (iter = &ipv4_protocols; *iter && *iter != proto;
	iter = &(*iter)->next);
    tp_assert(*iter);
    (*iter) = (*iter)->next;
    proto->next = NULL;
}

void ipv4_register_proto(ipv4_proto_t *proto)
{
    proto->next = ipv4_protocols;
    ipv4_protocols = proto;
}

void ipv4_uninit(void)
{
    ethernet_unregister_proto(&ipv4_proto);
}

void ipv4_init(void)
{
    ipv4_proto.eth_type = htons(ETHER_PROTOCOL_IP);
    ipv4_proto.recv = ipv4_recv;
    ethernet_register_proto(&ipv4_proto);
}
