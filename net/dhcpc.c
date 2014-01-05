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
#include <string.h> /* memcpy, memset */
#include "mem/tmalloc.h"
#include "util/tp_misc.h"
#include "net/dhcpc.h"
#include "net/packet.h"
#include "net/ether.h"
#include "net/ipv4.h"
#include "net/udp.h"
#include "net/net_debug.h"

#define DHCP_MIN_PACKET_LEN 300

#define BROADCAST_FLAG 0x8000
#define DHCP_MAGIC_COOKIE 0x63825363
#define DHCP_OP_REQUEST 1
#define DHCP_OP_REPLY 2
#define DHCP_HW_TYPE_ETH 1

#define DHCP_MSG_DISCOVER 1
#define DHCP_MSG_OFFER 2
#define DHCP_MSG_REQUEST 3
#define DHCP_MSG_ACK 5

typedef struct {
    udp_socket_t udp_sock;
    etherif_t *ethif;
    ipv4_info_t ip_info;
    u32 xid;
    u8 waited_message;
} dhcpc_t;

static u32 xid_seed = 0x453a939a;
static const u8 requested_options[] = { 0x1, 0x3 };

static int opt_put(u8 opt_num, const u8 opt[], u8 opt_len)
{
    u8 *p;

    if (!(p = packet_push(&g_packet, 2 + opt_len)))
	return -1;

    *p++ = opt_num;
    *p++ = opt_len;
    memcpy(p, opt, opt_len);
    return 0;
}

#define DECL_PUT_OPT(type) \
static int opt_put_##type(u8 opt_num, type val) \
{ \
    return opt_put(opt_num, (u8 *)&val, sizeof(type)); \
}

DECL_PUT_OPT(u8)
DECL_PUT_OPT(u16)
DECL_PUT_OPT(u32)

static int dhcpc_msg_xmit(dhcpc_t *dhcpc)
{
    dhcp_msg_t *msg;
    eth_mac_t mac;

    if (!(msg = packet_push(&g_packet, sizeof(dhcp_msg_t))))
	return -1;

    etherif_mac_addr_get(dhcpc->ethif, &mac);

    msg->op = DHCP_OP_REQUEST;
    msg->htype = DHCP_HW_TYPE_ETH;
    msg->hlen = 6;
    msg->hops = 0;
    msg->xid = dhcpc->xid;
    msg->secs = 0;
    msg->flags = htons(BROADCAST_FLAG);
    msg->ciaddr = 0;
    msg->yiaddr = 0;
    msg->siaddr = 0;
    msg->giaddr = 0;
    memcpy(msg->chaddr, mac.mac, 6);
    memset(msg->chaddr + 6, 0, 192 + 10);
    msg->magic_cookie = htonl(DHCP_MAGIC_COOKIE);
    return udp_sock_xmit(dhcpc->ethif, &dhcpc->udp_sock, &bcast_mac,
	IP_ADDR_ANY, IP_ADDR_BCAST, g_packet.length);
}

static int dhcpc_pad(void)
{
    u16 pad_len;

    pad_len = DHCP_MIN_PACKET_LEN - (g_packet.length + sizeof(dhcp_msg_t));

    if (!packet_push(&g_packet, pad_len))
	return -1;

    memset(g_packet.ptr, 0, pad_len);
    return 0;
}

static int dhcp_discover(dhcpc_t *dhcpc)
{
    packet_reset(&g_packet, PACKET_RESET_TAIL);

    /* Add options in reverse */
    if (opt_put_u8(0xff, 0) ||
	opt_put(55, requested_options, sizeof(requested_options)) ||
	opt_put_u16(57, htons(NET_PACKET_SIZE)) ||
	opt_put_u8(53, DHCP_MSG_DISCOVER))
    {
	return -1;
    }

    if (dhcpc_pad())
	return -1;

    dhcpc->xid = xid_seed++;
    return dhcpc_msg_xmit(dhcpc);
}

static int dhcp_request(dhcpc_t *dhcpc)
{
    packet_reset(&g_packet, PACKET_RESET_TAIL);

    /* Add options in reverse */
    if (opt_put_u8(0xff, 0) ||
	opt_put(55, requested_options, sizeof(requested_options)) ||
	opt_put_u16(57, htons(NET_PACKET_SIZE)) ||
	opt_put_u32(50, htonl(dhcpc->ip_info.ip)) ||
	opt_put_u8(53, DHCP_MSG_REQUEST))
    {
	return -1;
    }

    if (dhcpc_pad())
	return -1;

    return dhcpc_msg_xmit(dhcpc);
}

static int dhcpc_options_iter(int (*cb)(dhcpc_t *dhcpc, u8 opt, u8 len),
    dhcpc_t *dhcpc)
{
    while (g_packet.length >= 2)
    {
	u8 opt, len;

	opt = *(u8 *)g_packet.ptr;
	packet_pull(&g_packet, 1);
	len = *(u8 *)g_packet.ptr;
	packet_pull(&g_packet, 1);

	if  (opt == 0xFF)
	    break;

	if (cb(dhcpc, opt, len))
	    return -1;

	if (!packet_pull(&g_packet, len))
	{
	    tp_err(("Invalid DHCP option %d\n", opt));
	    return -1;
	}
    }

    return 0;
}

static int dhcpc_options_cb(dhcpc_t *dhcpc, u8 opt, u8 len)
{
#define VAL_U8(p) (*(u8 *)(p))
#define VAL_U32(p) (((u32)VAL_U8(p)) | ((u32)VAL_U8(p + 1) << 8) | \
    ((u32)VAL_U8(p + 2) << 16) | ((u32)VAL_U8(p + 3) << 24))

    switch (opt)
    {
    case 53:
	if (VAL_U8(g_packet.ptr) != dhcpc->waited_message)
	{
	    tp_err(("Expected %d, got %d\n",dhcpc->waited_message,
		VAL_U8(g_packet.ptr)));
	    return -1;
	}
	break;
    case 1:
	dhcpc->ip_info.netmask = ntohl(VAL_U32(g_packet.ptr));
	break;
    case 3:
	dhcpc->ip_info.router = ntohl(VAL_U32(g_packet.ptr));
	break;
    }
    return 0;
}

static int dhcpc_options_process(dhcpc_t *dhcpc)
{
    if (!packet_pull(&g_packet, sizeof(dhcp_msg_t)))
    {
	tp_err(("Not enough packet room for DHCP options"));
	return -1;
    }

    if (dhcpc_options_iter(dhcpc_options_cb, dhcpc))
    {
	tp_err(("DHCP options processing failed\n"));
	return -1;
    }

    return 0;
}

static void dhcpc_recv(udp_socket_t *sock)
{
    dhcpc_t *dhcpc = container_of(sock, dhcpc_t, udp_sock);
    dhcp_msg_t *msg = (dhcp_msg_t *)g_packet.ptr;
    eth_mac_t mac;

    tp_debug(("DHCP Received\n"));

    if (msg->op != DHCP_OP_REPLY || msg->htype != DHCP_HW_TYPE_ETH ||
	msg->hlen != 6 || msg->hops || msg->xid != dhcpc->xid)
    {
	return;
    }

    etherif_mac_addr_get(dhcpc->ethif, &mac);
    if (memcmp(msg->chaddr, mac.mac, 6))
	return;

    dhcpc->ip_info.ip = ntohl(msg->yiaddr);
    
    if (dhcpc_options_process(dhcpc))
	return;

    tp_out(("Address: %s\n", ip_addr_serialize(htonl(dhcpc->ip_info.ip))));
    tp_out(("Netmask: %s\n", ip_addr_serialize(htonl(dhcpc->ip_info.netmask))));
    tp_out(("Router: %s\n", ip_addr_serialize(htonl(dhcpc->ip_info.router))));

    switch (dhcpc->waited_message)
    {
    case DHCP_MSG_OFFER:
	tp_debug(("DHCP OFFER\n"));
	dhcpc->waited_message = DHCP_MSG_ACK;
	dhcp_request(dhcpc);
	break;
    case DHCP_MSG_ACK:
	tp_debug(("DHCP ACK\n"));
	etherif_ipv4_info_set(dhcpc->ethif, &dhcpc->ip_info); 
	break;
    }
}

void dhcpc_stop(etherif_t *ethif)
{
    dhcpc_t *dhcpc = ethif->dhcpc;
    
    if (!dhcpc)
	return;

    udp_unregister_socket(ethif, &dhcpc->udp_sock);
    tfree(dhcpc);
}

int dhcpc_start(etherif_t *ethif)
{
    dhcpc_t *dhcpc = tmalloc_type(dhcpc_t);

    dhcpc->udp_sock.recv = dhcpc_recv;
    dhcpc->udp_sock.local_port = 68;
    dhcpc->udp_sock.remote_port = 67;
    dhcpc->udp_sock.next = NULL;
    dhcpc->waited_message = DHCP_MSG_OFFER;

    udp_register_socket(ethif, &dhcpc->udp_sock);

    dhcpc->ethif = ethif;
    ethif->dhcpc = dhcpc;

    if (dhcp_discover(dhcpc))
    {
	dhcpc_stop(ethif);
	return -1;
    }

    return 0;
}
