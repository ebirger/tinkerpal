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
#include "net/udp.h"
#include "net/ipv4.h"
#include "net/packet.h"

static ipv4_proto_t udp_proto;

int udp_xmit(etherif_t *ethif, const eth_mac_t *dst_mac, u32 src_addr,
    u32 dst_addr, u16 src_port, u16 dst_port, u16 payload_len)
{
    udp_hdr_t *udph;
    u16 len;

    len = sizeof(udp_hdr_t) + payload_len;

    /* UDP Header */
    if (!(udph = packet_push(&g_packet, sizeof(udp_hdr_t))))
	return -1;

    udph->src_port = htons(src_port);
    udph->dst_port = htons(dst_port);
    udph->length = htons(len);
    udph->checksum = 0;

    return ipv4_xmit(ethif, dst_mac, IP_PROTOCOL_UDP, src_addr, dst_addr, len);
}

static void udp_recv(etherif_t *ethif)
{
    udp_hdr_t *udph = (udp_hdr_t *)g_packet.ptr;
    udp_socket_t *sock;
    u16 dst_port, src_port;

    tp_debug(("UDP packet received\n"));

    src_port = ntohs(udph->src_port);
    dst_port = ntohs(udph->dst_port);
    if (!src_port || !dst_port)
	return; /* Malformed header */

    for (sock = ethif->udp; sock; sock = sock->next)
    {
	if (sock->local_port != dst_port)
	    continue;

	if (sock->remote_port != UDP_PORT_ANY && sock->remote_port != src_port)
	    continue;

	/* Found match */
	break;
    }

    if (!sock)
    {
	tp_info(("No matching socket found\n"));
	return;
    }

    packet_pull(&g_packet, sizeof(udp_hdr_t));
    sock->recv(sock);
}

void udp_unregister_socket(etherif_t *ethif, udp_socket_t *sock)
{
    udp_socket_t **iter;

    for (iter = (udp_socket_t **)&ethif->udp; *iter && *iter != sock;
	iter = &(*iter)->next);
    tp_assert(*iter);
    (*iter) = (*iter)->next;
    sock->next = NULL;
}

void udp_register_socket(etherif_t *ethif, udp_socket_t *sock)
{
    sock->next = ethif->udp;
    ethif->udp = sock;
}

void udp_uninit(void)
{
    ipv4_unregister_proto(&udp_proto);
}

void udp_init(void)
{
    udp_proto.protocol = IP_PROTOCOL_UDP;
    udp_proto.recv = udp_recv;
    ipv4_register_proto(&udp_proto);
}
