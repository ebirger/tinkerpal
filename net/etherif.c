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
#include "util/debug.h"
#include "net/etherif.h"

static etherif_t *etherifs;
static int etherifs_last_id;

void etherif_uninit(etherif_t *ethif)
{
    etherif_t **iter;

    /* Unlink from list */
    for (iter = &etherifs; *iter && *iter != ethif; iter = &(*iter)->next);
    tp_assert(*iter);
    *iter = (*iter)->next;

    /* Remove events */
    event_watch_del(ethif->port_change_watch_id);
    event_watch_del(ethif->packet_received_watch_id);
    event_watch_del(ethif->packet_xmitted_watch_id);
}

void etherif_init(etherif_t *ethif, const etherif_ops_t *ops)
{
    ethif->ops = ops;
    ethif->id = etherifs_last_id++;

    ethif->port_change_watch_id = -1;
    ethif->packet_received_watch_id = -1;
    ethif->packet_xmitted_watch_id = -1;

    /* Link to list */
    ethif->next = etherifs;
    etherifs = ethif;
}

etherif_t *etherif_get_by_id(int id)
{
    etherif_t *ret;

    for (ret = etherifs; ret && ret->id != id; ret = ret->next);
    tp_assert(ret);
    return ret;
}
