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
#ifndef __TSTR_H__
#define __TSTR_H__

#include <string.h> /* memcmp */

typedef struct {
    char *value;
    unsigned short len;
#define TSTR_FLAG_ALLOCATED 0x0001
#define TSTR_FLAG_ESCAPED 0x0002
    unsigned short flags;
} tstr_t;

#define TSTR_IS_ALLOCATED(t) ((t)->flags & TSTR_FLAG_ALLOCATED)
#define TSTR_SET_ALLOCATED(t) ((t)->flags |= TSTR_FLAG_ALLOCATED)
#define TSTR_IS_ESCAPED(t) ((t)->flags & TSTR_FLAG_ESCAPED)
#define TSTR_SET_ESCAPED(t) ((t)->flags |= TSTR_FLAG_ESCAPED)
#define S(s) (tstr_t){ .value = (s), .len = sizeof(s) - 1, .flags = 0 }

char digit_value(char c);

typedef struct tstr_list_t {
    struct tstr_list_t *next;
    tstr_t str;
} tstr_list_t;

void tstr_init(tstr_t *t, char *s);

/* Allocate the data within the tstr */
void tstr_alloc(tstr_t *t, int len);

void tstr_list_add(tstr_list_t **l, tstr_t s);
void tstr_list_free(tstr_list_t **l);

static inline int tstr_cmp(const tstr_t *a, const tstr_t *b)
{
    return a->len != b->len || (b->len && 
	(*a->value != *b->value)) || 
	memcmp(a->value, b->value, b->len);
}

int tstr_find(tstr_t *haystack, tstr_t *needle);
tstr_t tstr_dup(tstr_t s);
void tstr_free(tstr_t *s);
void tstr_cat(tstr_t *dst, tstr_t *a, tstr_t *b);
void tstr_unescape(tstr_t *dst, tstr_t *src);
/* Returns a new tstr pointing to the old value, with len set
 * to the substring until delim
 * The prefix is cut from the original tstr.
 */
tstr_t tstr_cut(tstr_t *t, char delim);

/* Return a NULL terminated, allocated string from tstr_t */
char *tstr_to_strz(tstr_t *t);

static inline void tstr_advance(tstr_t *t, int amount)
{
    t->value += amount;
    t->len -= amount;
}

int prefix_comp(int len, char *a, char *b);

#endif
