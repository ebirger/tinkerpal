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
#ifndef __IPV4_H__
#define __IPV4_H__

#ifdef CONFIG_IPV4

#include "net/net_types.h"
#include "net/etherif.h"

typedef struct ipv4_proto_t ipv4_proto_t;

struct ipv4_proto_t {
    ipv4_proto_t *next;
    u16 protocol;
    void (*recv)(etherif_t *ethif);
};

/* - packet ptr is expected to point to the IPv4 payload
 * - addresses are in host order
 */
void ipv4_xmit(etherif_t *ethif, eth_mac_t *dst_mac, u8 protocol, u32 src_addr,
    u32 dst_addr, u16 payload_len);

/* protocols are assumed to be statically allocated */
void ipv4_unregister_proto(ipv4_proto_t *proto);
void ipv4_register_proto(ipv4_proto_t *proto);

void ipv4_uninit(void);
void ipv4_init(void);

#else

static inline void ipv4_uninit(void) { }
static inline void ipv4_init(void) { }

#endif

#endif
