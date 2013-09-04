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
#include "util/tstr.h"
#include "apps/history.h"

char *history_buf, *history_last;

void history_next(tstr_t *history)
{
    line_desc_t *line;

    /* Advance to the next node */
    line = ((line_desc_t *)history->value) - 1;
    history->value = line->next;

    /* Calculate length of currently iterated node by subtracting it from the
     * next node.
     */
    line = ((line_desc_t *)history->value) - 1;
    if (line->next)
    	history->len = line->next - history->value - sizeof(line_desc_t);
    else
    	history->len = 0;
}

void history_prev(tstr_t *history)
{
    line_desc_t *line;

    line = ((line_desc_t *)history->value) - 1;

    history->value = line->prev;
    history->len = (char *)line - line->prev;
}

#define ALIGN4(x) ((char *)(((unsigned long)(x) + 0x3) & ~0x3))

void history_commit(tstr_t *history, tstr_t *l)
{
    line_desc_t *line;
    char *next, *cur;
    
    cur = l->value;
    next = ALIGN4(cur + l->len + sizeof(line_desc_t));

    /* Set current node's next */
    line = ((line_desc_t *)l->value) - 1;
    line->next = next;

    /* Set next node's prev */
    line = ((line_desc_t *)next) - 1;
    line->prev = cur;

    /* Set next node's next */
    line->next = NULL;

    /* Advance l */
    history_last = l->value = next;

    /* Set history to new entry */
    history->value = history_last;
    history->len = l->len = 0;
}
