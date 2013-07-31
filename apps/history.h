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
#ifndef __HISTORY_H__
#define __HISTORY_H__

#include "util/tstr.h"
#include "util/tnum.h"

/* XXX: this should be moved once cli buffer is decoupled from history buffer */
typedef struct {
    char *next;
    char *prev;
} line_desc_t;

extern char *history_buf, *history_last;

static inline void history_init(tstr_t *history, char *buf)
{
    history_buf = history->value = buf;
    history->len = 0;
}

static inline int history_get(tstr_t *history, char *buf, int free_size)
{
    int size;
    
    size = MIN(history->len, free_size);
    memcpy(buf, history->value, size);
    return size;
}

static inline int history_is_first(tstr_t *history)
{
    return history->value == history_buf;
}

static inline int history_is_last(tstr_t *history)
{
    return history->value == history_last;
}

void history_next(tstr_t *history);
void history_prev(tstr_t *history);
void history_commit(tstr_t *history, tstr_t *l);

#endif
