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
#ifndef __NET_H__
#define __NET_H__

#ifdef CONFIG_NET

#include "net/net_debug.h"

#ifdef CONFIG_ETHERIF
#include "net/etherif.h"
#endif
#ifdef CONFIG_PACKET
#include "net/packet.h"
#endif
#ifdef CONFIG_ETHERNET
#include "net/ether.h"
#endif
#ifdef CONFIG_ARP
#include "net/arp.h"
#endif
#ifdef CONFIG_IPV4
#include "net/ipv4.h"
#endif
#ifdef CONFIG_ICMP
#include "net/icmp.h"
#endif
#ifdef CONFIG_UDP
#include "net/udp.h"
#endif
#ifdef CONFIG_DHCP_CLIENT
#include "net/dhcpc.h"
#endif

u16 net_csum(u16 *addr, u16 byte_len);

void net_uninit(void);
void net_init(void);

#else

static inline void net_uninit(void) { }
static inline void net_init(void) { }

#endif

#endif
