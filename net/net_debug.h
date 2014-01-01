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
#ifndef __NET_DEBUG_H__
#define __NET_DEBUG_H__

#include "net/net_types.h"

#ifdef CONFIG_NET_DEBUG

char *eth_mac_serialize(eth_mac_t *m);
/* Address is expected in network order */
char *ip_addr_serialize(u32 ip);
void eth_hdr_dump(eth_hdr_t *hdr);
void arp_packet_dump(arp_packet_t *arp);
void ip_hdr_dump(ip_hdr_t *iph);
void udp_hdr_dump(udp_hdr_t *udph);
void icmp_hdr_dump(icmp_hdr_t *icmph);
void dhcp_msg_dump(dhcp_msg_t *msg);

#else

static inline char *eth_mac_serialize(eth_mac_t *m) { return ""; }
static inline char *ip_addr_serialize(u32 ip) { return ""; }
static inline void eth_hdr_dump(eth_hdr_t *hdr) { }
static inline void arp_packet_dump(arp_packet_t *arp) { }
static inline void ip_hdr_dump(ip_hdr_t *iph) { }
static inline void udp_hdr_dump(udp_hdr_t *udph) { }
static inline void icmp_hdr_dump(icmp_hdr_t *icmph) { }
static inline void dhcp_msg_dump(dhcp_msg_t *msg) { }

#endif

#endif
