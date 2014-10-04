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
#include "net/netif.h"
#include "util/debug.h"

static netif_t *netifs;
static int netifs_last_id;

netif_t *netif_get_by_id(int id)
{
    netif_t *ret;

    for (ret = netifs; ret && ret->id != id; ret = ret->next);
    tp_assert(ret);
    return ret;
}

void netif_unregister(netif_t *netif)
{
    netif_event_t event;
    netif_t **iter;

    for (iter = &netifs; *iter && *iter != netif; iter = &(*iter)->next);
    tp_assert(*iter);
    *iter = (*iter)->next;
    
    /* Remove events */
    for (event = NETIF_EVENT_FIRST; event < NETIF_EVENT_COUNT; event++)
        event_watch_del_by_resource(NETIF_RES(netif, event));
}

void netif_register(netif_t *netif, const netif_ops_t *ops)
{
    netif->ops = ops;
    netif->id = netifs_last_id++;
    netif->next = netifs;
    netifs = netif;
}
