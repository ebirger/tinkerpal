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
#ifndef __NET_TYPES_H__
#define __NET_TYPES_H__

#include "util/tp_types.h"

#ifdef CONFIG_BIG_ENDIAN

#ifndef htonl
#define htonl(a) (a)
#endif

#ifndef htons
#define htons(a) (a)
#endif

#else

#ifndef htonl
#define htonl(a) \
    ((((a) >> 24) & 0x000000ff) | \
     (((a) >>  8) & 0x0000ff00) | \
     (((a) <<  8) & 0x00ff0000) | \
     (((a) << 24) & 0xff000000))
#endif

#ifndef ntohl
#define ntohl(a) htonl((a))
#endif

#ifndef htons
#define htons(a) \
    ((((a) >> 8) & 0x00ff) | \
     (((a) << 8) & 0xff00))
#endif

#ifndef ntohs
#define ntohs(a) htons((a))
#endif

#endif

#define ETHER_PROTOCOL_ARP 0x0806
#define ETHER_PROTOCOL_IP 0x0800

#define IP_PROTOCOL_UDP 17

typedef struct {
    u8 mac[6];
} eth_mac_t;

typedef struct {
    eth_mac_t dst;
    eth_mac_t src;
    u16 eth_type;
} eth_hdr_t;

typedef struct __attribute__((packed)) {
    u16 htype;
    u16 ptype;
    u8 hlen;
    u8 plen;
    u16 oper;
    eth_mac_t sha;
    u32 spa;
    eth_mac_t tha;
    u32 tpa;
} arp_packet_t;

typedef struct __attribute__((packed)) {
#ifdef CONFIG_BIG_ENDIAN_BITFIELD
    u8 ver : 4;
    u8 ihl : 4;
#else
    u8 ihl : 4;
    u8 ver : 4;
#endif
    u8 dscp;
    u16 tot_len;
    u16 id;
    u16 frag_off;
    u8 ttl;
    u8 protocol;
    u16 checksum;
    u32 src_addr;
    u32 dst_addr;
} ip_hdr_t;

typedef struct __attribute__((packed)) {
    u16 src_port;
    u16 dst_port;
    u16 length;
    u16 checksum;
} udp_hdr_t;

typedef struct __attribute__((packed)) {
    u8 op;
    u8 htype;
    u8 hlen;
    u8 hops;
    u32 xid;
    u16 secs;
    u16 flags;
    u32 ciaddr;
    u32 yiaddr;
    u32 siaddr;
    u32 giaddr;
    u8 chaddr[16];
    u8 sname[64];
    u8 boot_fname[128];
    u32 magic_cookie;
} dhcp_msg_t;

#endif
