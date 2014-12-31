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
#define TSTR_FLAG_INLINE 0x0008
    unsigned short flags;
    union {
        char *ptr;
        char buf[0];
    } u;
} tstr_t;

#define TPTR(t) ((t)->flags & TSTR_FLAG_INLINE ? (t)->u.buf : (t)->u.ptr)
#define TSTR_IS_ALLOCATED(t) ((t)->flags & TSTR_FLAG_ALLOCATED)
#define TSTR_SET_ALLOCATED(t) ((t)->flags |= TSTR_FLAG_ALLOCATED)
#define TSTR_IS_ESCAPED(t) ((t)->flags & TSTR_FLAG_ESCAPED)
#define TSTR_SET_ESCAPED(t) ((t)->flags |= TSTR_FLAG_ESCAPED)
#define TSTR_IS_INTERNAL(t) ((t)->flags & TSTR_FLAG_INTERNAL)
#define TSTR_SET_INTERNAL(t) ((t)->flags |= TSTR_FLAG_INTERNAL)
#define S(s) (tstr_t){ .u = { .ptr = (s) }, .len = sizeof(s) - 1, .flags = 0 }
#define INTERNAL_S(s) (tstr_t){ .u = { .ptr = (s) }, .len = sizeof(s) - 1, \
    .flags = TSTR_FLAG_INTERNAL }

/** @brief Initialize a tstr_t instance
 *
 * @param t the tstr_t to initialize
 * @param data data buffer
 * @param len size of the data
 * @param flags tstr_t flags
 * @return void
 */
void tstr_init(tstr_t *t, char *data, int len, unsigned short flags);

/** @brief Initialize a tstr_t instance and allocate associated data
 *
 * tstr_t data may be allocated using tmalloc, or may be using the tstr_t
 * structure itself (i.e. TSTR_FLAG_INLINE)
 *
 * @param t the tstr_t to initialize
 * @param len size to allocate
 * @return void
 */
void tstr_init_alloc_data(tstr_t *t, int len);

/** @brief Initialize a tstr_t instance, allocate and zero associated data
 *
 * tstr_t data may be allocated using tmalloc, or may be using the tstr_t
 * structure itself (i.e. TSTR_FLAG_INLINE)
 *
 * @param t the tstr_t to initialize
 * @param len size to allocate
 * @return void
 */
void tstr_init_zalloc_data(tstr_t *t, int len);

/** @brief Initialize a tstr_t instance, copy data content from C string
 *
 * tstr_t data may be allocated using tmalloc, or may be using the tstr_t
 * structure itself (i.e. TSTR_FLAG_INLINE)
 * Data content is copied from C string
 *
 * @param t the tstr_t to initialize
 * @param s C string to copy
 * @return void
 */
void tstr_init_copy_string(tstr_t *t, const char *s);

/** @brief Free a tstr_t instance
 *
 * @param t the tstr_t to free
 * @return void
 */
void tstr_free(tstr_t *s);

/** @brief Compare two tstr_t instances
 *
 * @param a first tstr_t to compare
 * @param b second tstr_t to compare
 * @return 0 if equal, > 0 if tstr_t 'b' precedes tstr_t 'a', < 0 otherwise 
 */
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

/** @brief Compare a tstr_t to a C string
 *
 * Strings must match in length
 *
 * @param a first tstr_t to compare
 * @param b C string to compare
 * @param blen exact number of characters to compare
 * @return 0 if equal, non zero otherwise
 */
static inline int _tstr_cmp_str(const tstr_t *a, const char *b,
    unsigned short blen)
{
    return a->len != blen || (blen && (*TPTR(a) != *b)) ||
        memcmp(TPTR(a), b, blen);
}

/** @brief Compare a tstr_t to a C string
 *
 * Strings must match in length
 *
 * @param a first tstr_t to compare
 * @param b C string to compare
 * @return 0 if equal, non zero otherwise
 */
static inline int tstr_cmp_str(const tstr_t *a, const char *b)
{
    unsigned short blen = strlen(b);

    return _tstr_cmp_str(a, b, blen);
}

/** @brief Compare a tstr_t to a C string up to a limited length
 *
 * @param a first tstr_t to compare
 * @param b C string to compare
 * @param blen maximal number of characters to compare
 * @return 0 if equal, non zero otherwise
 */
static inline int tstr_ncmp_str(const tstr_t *a, const char *b,
    unsigned short blen)
{
    return a->len < blen || (blen && (*TPTR(a) != *b)) ||
        memcmp(TPTR(a), b, blen);
}

/** @brief Lookup an instance of a tstr_t in another tstr_t
 *
 * @param haystack tstr_t to look in
 * @param needle tstr_t to look for
 * @return -1 if LEN(haystack) == 0, 1 if LEN(needle) == 0, index of needle in
 *     haystack otherwise
 */
int tstr_find(const tstr_t *haystack, tstr_t *needle);

/** @brief Return a duplicate tstr_t instance
 *
 * tstr_t is duplicated if contains an allocated buffer
 *
 * @param s tstr_t to duplicate
 * @return duplicate tstr_t
 */
tstr_t tstr_dup(tstr_t s);

/** @brief Return a tstr_t pointing to s[index] - with count bytes
 *
 * New tstr_t data may point to the original tstr_t data
 *
 * @param s tstr_t to slice
 * @param index index to start from
 * @param count number of characters to slice
 * @return sliced tstr_t
 */
tstr_t tstr_piece(const tstr_t *s, int index, int count);

/** @brief Return a tstr_t pointing to s[index] - with count bytes
 *
 * New tstr_t data never points to the original tstr_t data
 *
 * @param s tstr_t to slice
 * @param index index to start from
 * @param count number of characters to slice
 * @return sliced tstr_t
 */
static inline tstr_t tstr_slice(const tstr_t *s, int index, int count)
{
    return tstr_dup(tstr_piece(s, index, count));
}

/** @brief Concatenate two tstr_t instances contents into a new tstr_t
 *
 * New tstr_t data never points to the original tstr_t data
 *
 * @param dst output tstr_t
 * @param a first tstr_t
 * @param b second tstr_t
 * @return void
 */
void tstr_cat(tstr_t *dst, tstr_t *a, tstr_t *b);

/** @brief Unescape tstr_t contents
 *
 * New tstr_t data never points to the original tstr_t data
 *
 * @param dst output tstr_t
 * @param src input tstr_t
 * @return void
 */
void tstr_unescape(tstr_t *dst, tstr_t *src);

/** @brief Convert tstr_t to all upper-case/lower-case
 *
 * New tstr_t data never points to the original tstr_t data
 *
 * @param s input tstr_t
 * @param is_lower 0 for converting to upper-case, 1 for lower-case
 * @return new tstr_t
 */
tstr_t tstr_to_upper_lower(tstr_t s, int is_lower);

/** @brief Return a NULL terminated, allocated C string from tstr_t
 *
 * @param t input tstr_t
 * @return C string
 */
char *tstr_to_strz(tstr_t *t);

/** @brief Dump content of tstr_t calling a user provided callback function
 *
 * This version receives a user provided context to be passed to the callback
 *
 * @param t input tstr_t
 * @param offset offset in tstr_t to start from
 * @param size number of characters to dump
 * @param dump_fn user provided callback
 * @param ctx user provided context to be passed to dump_fn
 * @return 0 on succes, non-zero otherwise
 */
int __tstr_dump(tstr_t *t, int offset, int size,
    int (*__dump_fn)(void *ctx, char *buf, int size), void *ctx);

/** @brief Dump content of tstr_t calling a user provided callback function
 *
 * This version does not receive a user provided context to be passed to the
 *     callback
 *
 * @param t input tstr_t
 * @param offset offset in tstr_t to start from
 * @param size number of characters to dump
 * @param dump_fn user provided callback
 * @return 0 on succes, non-zero otherwise
 */
int tstr_dump(tstr_t *t, int offset, int size,
    int (*dump_fn)(char *buf, int size));

/** @brief Fetch content of tstr_t instance at offset
 *
 * @param t input tstr_t
 * @param offset offset to peek at
 * @return character at t[offset]
 */
static inline char tstr_peek(const tstr_t *t, int offset)
{
    return *(TPTR(t) + offset);
}

/** @brief Fill content of tstr_t from a user provided callback function
 *
 * tstr_t instance data must be pre-allocated to fit the required size
 *
 * @param t tstr_t
 * @param size number of characters to fetch
 * @param fill_fn function for filling tstr_t
 * @param ctx context to be passed to fill_fn
 * @return 0 on succes, non-zero otherwise
 */
int tstr_fill(tstr_t *t, int size,
    int (*fill_fn)(void *ctx, char *buf, int size), void *ctx);

/** @brief Move tstr_t data contents
 *
 * @param t input tstr_t
 * @param to_idx index to move to
 * @param from_idx index to move from
 * @param count number of characters to move
 * @return void
 */
void tstr_move(tstr_t *t, int to_idx, int from_idx, int count);

/** @brief Copy tstr_t content into a buffer
 *
 * @param buf output buffer
 * @param t input tstr_t
 * @param offset offset in t to start copying from
 * @param size number of characters to copy
 * @return void
 */
static inline void tstr_serialize(char *buf, const tstr_t *t, int offset,
    int size)
{
    memcpy(buf, TPTR(t) + offset, size);
}

/** @brief Copy buffer content into a tstr_t instance
 *
 * tstr_t data must be pre-allocated, all other tstr_t fields are kept
 *
 * @param t output tstr_t
 * @param buf input buffer
 * @param offset offset in t to start copying into
 * @param size number of characters to copy
 * @return void
 */
static inline void tstr_cpy_buf(tstr_t *t, char *buf, int offset, int size)
{
    memcpy(TPTR(t) + offset, buf, size);
}

/** @brief Split tstr_t into two tstr_t instances
 *
 * Split tstr_t t as:
 *     +--------------------------------+
 *     |          t data content        |
 *     +-----------+-----+--------------+
 *     |  a        | sep |  b           |
 *     +-----------+-----+--------------+
 *
 * tstr_t instances data is newly allocated
 *
 * @param t input tstr_t
 * @param a first output tstr_t to initialize
 * @param b second output tstr_t to initialize
 * @param idx offset in t of separator
 * @param sep_len size of separator
 * @return void
 */
static inline void tstr_split(const tstr_t *t, tstr_t *a, tstr_t *b, int idx,
    int sep_len)
{
    *a = tstr_piece(t, 0, idx);
    *b = tstr_piece(t, idx + sep_len, t->len - (idx + sep_len));
}

#endif
