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
#include "platform/arm/stellaris/stellaris_eth.h"
#include "util/tp_types.h"
#include "util/tp_misc.h"
#include "util/debug.h"
#include "util/event.h"
#include "mem/tmalloc.h"

typedef struct {
    etherif_t ethif;
} stellaris_eth_t;

#define ETHIF_TO_STELLARIS_ETH(x) container_of(x, stellaris_eth_t, ethif)

static int stellaris_eth_link_status(etherif_t *ethif)
{
    return 1;
}

static int stellaris_eth_packet_size(etherif_t *ethif)
{
    return 0;
}

static int stellaris_eth_packet_recv(etherif_t *ethif, u8 *buf, int size)
{
    return 0;
}

static void stellaris_eth_packet_xmit(etherif_t *ethif, u8 *buf, int size)
{
}

void stellaris_eth_free(etherif_t *ethif)
{
    stellaris_eth_t *se = ETHIF_TO_STELLARIS_ETH(ethif);

    etherif_uninit(ethif);
    tfree(se);
}

static const etherif_ops_t stellaris_eth_etherif_ops = {
    .link_status = stellaris_eth_link_status,
    .packet_size = stellaris_eth_packet_size,
    .packet_recv = stellaris_eth_packet_recv,
    .packet_xmit = stellaris_eth_packet_xmit,
};

etherif_t *stellaris_eth_new(void)
{
    stellaris_eth_t *se = tmalloc_type(stellaris_eth_t);

    etherif_init(&se->ethif, &stellaris_eth_etherif_ops);

    return &se->ethif;
}
