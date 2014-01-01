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
#ifndef __UDP_H__
#define __UDP_H__

#include "net/etherif.h"

typedef struct udp_socket_t udp_socket_t;

#define UDP_PORT_ANY 0

struct udp_socket_t {
    udp_socket_t *next;
    etherif_t *ethif;
    u16 local_port; /* host order */
    u16 remote_port; /* host order */
    void (*recv)(udp_socket_t *sock);
};

/* - packet ptr is expected to point to the UDP payload
 * - ports and addresses are in host order
 */
int udp_xmit(etherif_t *ethif, const eth_mac_t *dst_mac, u32 src_addr,
    u32 dst_addr, u16 src_port, u16 dst_port, u16 payload_len);

int udp_socket_xmit(udp_socket_t *sock, const eth_mac_t *dst_mac, u32 src_addr,
    u32 dst_addr, u16 payload_len);

/* Sockets are assumed to be statically allocated */
void udp_unregister_socket(udp_socket_t *sock);
void udp_register_socket(udp_socket_t *sock);

void udp_uninit(void);
void udp_init(void);

#endif
