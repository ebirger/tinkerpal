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
#include "util/tstr.h"
#include "apps/history.h"

/* XXX: allow removing old entries upon:
 *   1. newer entries
 *   2. Memory shortage
 */

typedef struct {
    char *next;
    char *prev;
    int len;
} line_desc_t;

struct history_t {
    tstr_t current;
    char *buf;
    int free_size;
    char *last;
};

history_t g_history; /* Singleton for now */

void history_next(history_t *h)
{
    line_desc_t *line;

    /* Advance to the next node */
    line = ((line_desc_t *)h->current.value) - 1;
    h->current.value = line->next;

    /* Extract length */
    line = ((line_desc_t *)h->current.value) - 1;
    h->current.len = line->len;
}

void history_prev(history_t *h)
{
    line_desc_t *line;

    /* Go back to the previous node */
    line = ((line_desc_t *)h->current.value) - 1;
    h->current.value = line->prev;

    /* Extract length */
    line = ((line_desc_t *)h->current.value) - 1;
    h->current.len = line->len;
}

#define ALIGN4(x) ((char *)(((unsigned long)(x) + 0x3) & ~0x3))

void history_commit(history_t *h, tstr_t *l)
{
    line_desc_t *line;
    char *next, *cur;
    
    if (l->len + sizeof(line_desc_t) > h->free_size)
    {
	/* XXX: Should overrun old entries */
	tp_warn(("History: no more room\n"));
	return;
    }

    /* Copy line content */
    cur = h->current.value;
    memcpy(cur, l->value, l->len);

    /* Calculate next node 
     * XXX: make sure we don't exceed our limit
     */
    next = ALIGN4(cur + l->len + sizeof(line_desc_t));

    /* Set current node's next */
    line = ((line_desc_t *)cur) - 1;
    line->len = l->len;
    line->next = next;

    /* Set next node's prev */
    line = ((line_desc_t *)next) - 1;
    line->prev = cur;

    /* Set next node's next */
    line->next = NULL;

    /* Set history to new entry */
    h->last = next;
    h->current.value = h->last;
    h->current.len = 0;

    h->free_size -= l->len + sizeof(line_desc_t);
}

int history_get(history_t *h, char *buf, int free_size)
{
    int size;
    
    size = MIN(h->current.len, free_size);
    memcpy(buf, h->current.value, size);
    return size;
}

int history_is_first(history_t *h)
{
    return h->current.value == h->buf;
}

int history_is_last(history_t *h)
{
    return h->current.value == h->last;
}

history_t *history_new(char *buf, int size)
{
    history_t *h = &g_history;

    h->buf = h->current.value = buf + sizeof(line_desc_t);
    h->free_size = size - sizeof(line_desc_t);
    h->current.len = 0;
    return h;
}

void history_free(history_t *h)
{
    /* Nothing for now. Singleton */
}
