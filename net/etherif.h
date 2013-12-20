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
#ifndef __ETHERIF_H__
#define __ETHERIF_H__

#include "util/event.h"
#include "util/tp_types.h"

typedef struct etherif_t etherif_t;

typedef struct {
    int (*link_status)(etherif_t *e);
    int (*packet_size)(etherif_t *e);
    int (*packet_recv)(etherif_t *e, u8 *buf, int size);
    void (*packet_xmit)(etherif_t *e, u8 *buf, int size);
} etherif_ops_t;

struct etherif_t {
    const etherif_ops_t *ops;
    event_t *on_port_change;
    event_t *on_packet_received;
    event_t *on_packet_xmit;
};

static inline void etherif_init(etherif_t *ethif, const etherif_ops_t *ops)
{
    ethif->ops = ops;
    ethif->on_port_change = NULL;
    ethif->on_packet_received = NULL;
    ethif->on_packet_xmit = NULL;
}

static inline void etherif_on_port_change_event_set(etherif_t *ethif,
    event_t *ev)
{
    ethif->on_port_change = ev;
}

static inline void etherif_on_packet_received_event_set(etherif_t *ethif,
    event_t *ev)
{
    ethif->on_packet_received = ev;
}

static inline void etherif_on_packet_xmit_event_set(etherif_t *ethif,
    event_t *ev)
{
    ethif->on_packet_xmit = ev;
}

#endif
