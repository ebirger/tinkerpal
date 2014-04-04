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
#include "net/net_types.h"
#include "net/net_debug.h"
#include "net/icmp.h"
#include "net/ipv4.h"
#include "net/net.h"
#include "net/packet.h"

#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO_REQUEST 8

static ipv4_proto_t icmp_proto;

static void icmp_echo_req_recv(etherif_t *ethif)
{
    icmp_hdr_t *icmph = (icmp_hdr_t *)g_packet.ptr;
    ip_hdr_t *iph;
    eth_hdr_t *eth_hdr;
    eth_mac_t *dst_mac;
    u32 src_addr, dst_addr;

    if (!ipv4_addr(ethif))
        return;

    /* Trick - fetch IP & Ethernet addresses from the packet */
    iph = packet_push(&g_packet, sizeof(ip_hdr_t));
    src_addr = ntohl(iph->dst_addr);
    dst_addr = ntohl(iph->src_addr);

    eth_hdr = packet_push(&g_packet, sizeof(eth_hdr_t));
    dst_mac = &eth_hdr->src;
    
    packet_pull(&g_packet, sizeof(eth_hdr_t));
    packet_pull(&g_packet, sizeof(ip_hdr_t));
   
    /* Construct ICMP reply */ 
    icmph->type = ICMP_ECHO_REPLY;
    icmph->code = 0;
    icmph->checksum = net_csum((u16 *)icmph, g_packet.length);

    ipv4_xmit(ethif, dst_mac, IP_PROTOCOL_ICMP, src_addr, dst_addr,
        g_packet.length);
}

static void icmp_recv(etherif_t *ethif)
{
    icmp_hdr_t *icmph = (icmp_hdr_t *)g_packet.ptr;

    tp_debug(("ICMP packet received\n"));

    switch (icmph->type)
    {
    case ICMP_ECHO_REQUEST:
        icmp_echo_req_recv(ethif);
        break;
    default:
        tp_warn(("unsupported ICMP message type %d\n", icmph->type));
        break;
    }
}

void icmp_uninit(void)
{
    ipv4_unregister_proto(&icmp_proto);
}

void icmp_init(void)
{
    icmp_proto.protocol = IP_PROTOCOL_ICMP;
    icmp_proto.recv = icmp_recv;
    ipv4_register_proto(&icmp_proto);
}
