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
#include "util/tp_types.h"
#include "util/debug.h"
#include "util/tstr.h"
#include "util/history.h"
#include "mem/tmalloc.h"

/* History is kept in a double linked list. There is always one empty item at
 * the end of the list.
 * First item->prev and last item->next point to themselves.
 */

typedef struct history_item_t {
    struct history_item_t *next;
    struct history_item_t *prev;
    tstr_t str;
} history_item_t;

struct history_t {
    mem_squeezer_t squeezer;
    history_item_t *items;
    history_item_t *current;
};

history_t g_history; /* Singleton for now */

void history_next(history_t *h)
{
    h->current = h->current->next;
}

void history_prev(history_t *h)
{
    h->current = h->current->prev;
}

void history_commit(history_t *h, tstr_t *l)
{
    history_item_t *item = tmalloc_type(history_item_t), *last;

    item->str = tstr_dup(*l);

    if (!h->items)
    {
        item->next = item->prev = h->items = h->current = item;
        return;
    }

    for (last = h->items; last->next != last; last = last->next);
    if (last == h->items)
    {
        h->items = item;
        item->prev = item;
    }
    else
    {
        item->prev = last->prev;
        last->prev->next = item;
    }
    last->prev = item;
    item->next = last;
    h->current = last;
}

int history_get(history_t *h, char *buf, int free_size)
{
    int size;
    
    size = MIN(h->current->str.len, free_size);
    tstr_serialize(buf, &h->current->str, 0, size);
    return size;
}

void history_dump(history_t *h)
{
    history_item_t *iter;
    int i = 0;

    for (iter = h->items; iter->next != iter; iter = iter->next)
        tp_out("%d: %S\n", i++, &iter->str);
}

static void history_item_free(history_item_t *i)
{
    tstr_free(&i->str);
    tfree(i);
}

static int history_squeeze(mem_squeezer_t *squeezer, int size)
{
    history_t *h = (history_t *)squeezer;
    history_item_t *item = h->items;
    int orig_size = size;

    tp_debug("history squeeze: asked to squeeze %d\n", size);
    while (item->next != item && size > 0)
    {
        history_item_t *tmp = item;
        
        item = item->next;
        if (!TSTR_IS_ALLOCATED(&tmp->str))
            continue;

        if (tmp == h->items)
        {
            tmp->next->prev = tmp->next;
            h->items = tmp->next;
        }
        else
        {
            tmp->next->prev = tmp->prev;
            tmp->prev->next = tmp->next;
        }

        size -= tmp->str.len;
        history_item_free(tmp);
    }

    return orig_size - size;
}

history_t *history_new(void)
{
    /* Add dummy tail */
    history_commit(&g_history, &S(""));
    g_history.squeezer.squeeze = history_squeeze;
    tmalloc_register_squeezer(&g_history.squeezer);
    return &g_history;
}

void history_free(history_t *h)
{
    history_item_t *tmp;

    tmalloc_unregister_squeezer(&g_history.squeezer);
    while ((tmp = h->items))
    {
        h->items = tmp->next == tmp ? NULL : tmp->next;
        history_item_free(tmp);
    }
}
