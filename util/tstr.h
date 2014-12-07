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

#include "util/tp_misc.h"
#include <string.h> /* memcmp */

typedef struct {
    unsigned short len;
#define TSTR_FLAG_ALLOCATED 0x0001
#define TSTR_FLAG_ESCAPED 0x0002
#define TSTR_FLAG_INTERNAL 0x0004
    unsigned short flags;
    char *ptr;
} tstr_t;

#define TPTR(t) ((t)->ptr)
#define TSTR_IS_ALLOCATED(t) ((t)->flags & TSTR_FLAG_ALLOCATED)
#define TSTR_SET_ALLOCATED(t) ((t)->flags |= TSTR_FLAG_ALLOCATED)
#define TSTR_IS_ESCAPED(t) ((t)->flags & TSTR_FLAG_ESCAPED)
#define TSTR_SET_ESCAPED(t) ((t)->flags |= TSTR_FLAG_ESCAPED)
#define TSTR_IS_INTERNAL(t) ((t)->flags & TSTR_FLAG_INTERNAL)
#define TSTR_SET_INTERNAL(t) ((t)->flags |= TSTR_FLAG_INTERNAL)
#define S(s) (tstr_t){ .ptr = (s), .len = sizeof(s) - 1, .flags = 0 }
#define INTERNAL_S(s) (tstr_t){ .ptr = (s), .len = sizeof(s) - 1, \
    .flags = TSTR_FLAG_INTERNAL }

char digit_value(char c);

void tstr_cpy_str(tstr_t *t, const char *s);

/* Allocate the data within the tstr */
void tstr_alloc(tstr_t *t, int len);
void tstr_zalloc(tstr_t *t, int len);

void tstr_init(tstr_t *t, char *data, int len, unsigned short flags);

static inline int tstr_cmp(const tstr_t *a, const tstr_t *b)
{
    int common_len, diff;
    char first_char_diff;

    if (!a->len && !b->len)
        return 0;

    if (!a->len || !b->len)
        return a->len - b->len;

    if ((first_char_diff = *TPTR(a) - *TPTR(b)))
        return first_char_diff;

    common_len = MIN(a->len, b->len);
    diff = memcmp(TPTR(a), TPTR(b), common_len);
    return diff ? : a->len - b->len;
}

static inline int _tstr_cmp_str(const tstr_t *a, const char *b,
    unsigned short blen)
{
    return a->len != blen || (blen && (*TPTR(a) != *b)) ||
        memcmp(TPTR(a), b, blen);
}

static inline int tstr_ncmp_str(const tstr_t *a, const char *b,
    unsigned short blen)
{
    return a->len < blen || (blen && (*TPTR(a) != *b)) ||
        memcmp(TPTR(a), b, blen);
}

static inline int tstr_cmp_str(const tstr_t *a, const char *b)
{
    unsigned short blen = strlen(b);

    return _tstr_cmp_str(a, b, blen);
}

int tstr_find(tstr_t *haystack, tstr_t *needle);
tstr_t tstr_dup(tstr_t s);
/* Return a tstr_t pointing to s[index] - with count bytes */
tstr_t tstr_piece(tstr_t s, int index, int count);
/* Return a copy of the tstr_piece */
static inline tstr_t tstr_slice(tstr_t s, int index, int count)
{
    return tstr_dup(tstr_piece(s, index, count));
}
void tstr_free(tstr_t *s);
void tstr_cat(tstr_t *dst, tstr_t *a, tstr_t *b);
void tstr_unescape(tstr_t *dst, tstr_t *src);

tstr_t tstr_to_upper_lower(tstr_t s, int is_lower);

/* Returns a new tstr pointing to the old value, with len set
 * to the substring until delim
 * The prefix is cut from the original tstr.
 */
tstr_t tstr_cut(tstr_t *t, char delim);

/* Return a NULL terminated, allocated string from tstr_t */
char *tstr_to_strz(tstr_t *t);

static inline void tstr_advance(tstr_t *t, int amount)
{
    TPTR(t) += amount;
    t->len -= amount;
}

int __tstr_dump(tstr_t *t, int offset, int size,
    int (*__dump_fn)(void *ctx, char *buf, int size), void *ctx);
int tstr_dump(tstr_t *t, int offset, int size,
    int (*dump_fn)(char *buf, int size));

static inline char tstr_peek(const tstr_t *t, int index)
{
    return *(TPTR(t) + index);
}

void tstr_move(tstr_t *t, int to_idx, int from_idx, int count);

int tstr_fill(tstr_t *t, int size,
    int (*fill_fn)(void *ctx, char *buf, int size), void *ctx);

static inline void tstr_serialize(char *buf, const tstr_t *t, int offset,
    int size)
{
    memcpy(buf, TPTR(t) + offset, size);
}

static inline void tstr_cpy_buf(tstr_t *t, char *buf, int offset, int size)
{
    memcpy(TPTR(t) + offset, buf, size);
}

#endif
