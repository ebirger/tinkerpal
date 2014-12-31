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
#include <stdio.h>
#include <string.h>
#include "util/tstr.h"
#include "util/debug.h"
#include "util/tp_misc.h"
#include "mem/tmalloc.h"

void tstr_init_alloc_data(tstr_t *t, int len)
{
    t->len = len;
    t->flags = 0;
    if (len <= sizeof(t->u))
        t->flags |= TSTR_FLAG_INLINE;
    else
    {
        t->u.ptr = tmalloc(len, "TSTR");
        TSTR_SET_ALLOCATED(t);
    }
}

void tstr_init_zalloc_data(tstr_t *t, int len)
{
    tstr_init_alloc_data(t, len);
    memset(TPTR(t), 0, len);
}

void tstr_init(tstr_t *t, char *data, int len, unsigned short flags)
{
    t->u.ptr = data;
    t->len = len;
    t->flags = flags;
}

void tstr_cpy_str(tstr_t *t, const char *s)
{
    int len = strlen(s);

    tstr_init_alloc_data(t, len);
    memcpy(TPTR(t), s, len);
}

int tstr_find(const tstr_t *haystack, tstr_t *needle)
{ 
    int i, j;

    if (haystack->len == 0)
        return -1;

    /* Trick for string.prototype.split() */
    if (needle->len == 0 && haystack->len > 0)
        return 1;

    /* Stupid brute force algorithm here... */
    for (i = 0; i <= haystack->len - needle->len; i++)
    {
        int diff = 0;

        for (j = 0; j < needle->len && !diff; j++)
        {
            if (TPTR(haystack)[i + j] != TPTR(needle)[j])
                diff = 1;
        }
        if (!diff)
            return i;
    }
    return -1;
}

tstr_t tstr_dup(tstr_t s)
{
    tstr_t ret;

    if (TSTR_IS_ALLOCATED(&s))
    {
        tstr_init_alloc_data(&ret, s.len);
        memcpy(TPTR(&ret), TPTR(&s), ret.len);
    }
    else
        ret = s;
    return ret;
}

tstr_t tstr_piece(const tstr_t *s, int index, int count)
{
    tstr_t ret;

    ret = *s;
    if (s->flags & TSTR_FLAG_INLINE)
        memmove(ret.u.buf, ret.u.buf + index, count);
    else
        ret.u.ptr += index;
    ret.len = count;
    return ret;
}

void tstr_free(tstr_t *s)
{
    if (TSTR_IS_ALLOCATED(s))
        tfree(TPTR(s));
}

void tstr_cat(tstr_t *dst, tstr_t *a, tstr_t *b)
{
    tstr_init_alloc_data(dst, a->len + b->len);
    memcpy(TPTR(dst), TPTR(a), a->len);
    memcpy(TPTR(dst) + a->len, TPTR(b), b->len);
}

void tstr_unescape(tstr_t *dst, tstr_t *src)
{
    unsigned short left;
    char *in, *out;

    out = tmalloc(src->len, "Unescaped string");
    tstr_init(dst, out, src->len, TSTR_FLAG_ALLOCATED);

    for (in = TPTR(src); (left = src->len - (in - TPTR(src))) > 0; in++)
    {
        if (*in != '\\')
        {
            *out++ = *in;
            continue;
        }
        in++;
        switch (*in)
        {
        case 'n': *out++ = '\n'; break;
        case 't': *out++ = '\t'; break;
        case 'r': *out++ = '\r'; break;
        case '\\': *out++ = '\\'; break;
        case '0': *out++ = '\0'; break;
        case '\'': *out++ = '\''; break;
        case '"': *out++ = '"'; break;
        case 'u':
            {
                char c;

                if (left < 5)
                {
                    /* Malformed, copy as is */
                    *out++ = *in;
                    break;

                }
                /* We don't really support unicode. Only \u00xx patterns where
                 * xx is a hex number.
                 */
                in++; /* Skip 'u'*/
                in++; /* Skip 0 */
                in++; /* Skip 0 */
                c = digit_value(*in++) * 16;
                c += digit_value(*in);
                *out++ = c;
                break;
            }
        default:
            /* Unknown, copy as is */
            *out++ = *in;
            break;
        }
    }
    dst->len = out - TPTR(dst);
}

tstr_t tstr_to_upper_lower(tstr_t s, int is_lower)
{
    char *out;
    tstr_t ret;
    int idx = 0;

    tstr_init_alloc_data(&ret, s.len);
    out = TPTR(&ret);
    while (s.len - idx)
    {
        u8 c = (u8)*(TPTR(&s) + idx++);

        *out++ = is_lower ? (char)tolower(c) : (char)toupper(c);
    }

    return ret;
}

char *tstr_to_strz(tstr_t *t)
{
    char *ret;

    ret = tmalloc(t->len + 1, "Null terminated string");
    memcpy(ret, TPTR(t), t->len);
    ret[t->len] = '\0';
    return ret;
}

int __tstr_dump(tstr_t *t, int offset, int size,
    int (*__dump_fn)(void *ctx, char *buf, int size), void *ctx)
{
    return __dump_fn(ctx, TPTR(t) + offset, size);
}

static int tstr_dump_dump_fn(void *ctx, char *buf, int size)
{
    int (*real_dump_fn)(char *buf, int size) = ctx;

    return real_dump_fn(buf, size);
}

int tstr_dump(tstr_t *t, int offset, int size,
    int (*dump_fn)(char *buf, int size))
{
    return __tstr_dump(t, offset, size, tstr_dump_dump_fn, dump_fn);
}

void tstr_move(tstr_t *t, int to_idx, int from_idx, int count)
{
    char *from, *to;

    tp_assert(from_idx > 0 && from_idx < t->len);
    tp_assert(to_idx > 0 && to_idx < t->len);

    from = TPTR(t) + from_idx;
    to = TPTR(t) + to_idx;
    memmove(to, from, count);
}

int tstr_fill(tstr_t *t, int size,
    int (*fill_fn)(void *ctx, char *buf, int size), void *ctx)
{
    return fill_fn(ctx, TPTR(t), size);
}
