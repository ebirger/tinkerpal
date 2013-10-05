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
#include <ctype.h>
#include "util/tstr.h"
#include "util/tmalloc.h"

/* XXX: move to a different file */
char digit_value(char c)
{
    if (isdigit((int)c))
	return c - '0';
    if (c >= 'a' && c <= 'f')
	return 10 + c - 'a';
    if (c >= 'A' && c <= 'F')
	return 10 + c - 'A';
    return 0;
}

void tstr_alloc(tstr_t *t, int len)
{
    TPTR(t) = tmalloc(len, "TSTR");
    t->len = len;
    t->flags = 0;
    TSTR_SET_ALLOCATED(t);
}

void tstr_init(tstr_t *t, char *data, int len, unsigned short flags)
{
    TPTR(t) = data;
    t->len = len;
    t->flags = flags;
}

void tstr_cpy_str(tstr_t *t, char *s)
{
    int len = strlen(s);

    tstr_alloc(t, len);
    memcpy(TPTR(t), s, len);
}

void tstr_list_add(tstr_list_t **l, tstr_t *s)
{
    tstr_list_t *n = tmalloc_type(tstr_list_t), **iter;
    /* TODO: verify no double strs */
    n->str = *s;
    n->next = NULL;
    for (iter = l; *iter; iter = &(*iter)->next);
    *iter = n;
}

void tstr_list_free(tstr_list_t **l)
{
    tstr_list_t *temp;

    while ((temp = *l))
    {
	*l = (*l)->next;
	tstr_free(&temp->str);
	tfree(temp);
    }
}

int tstr_find(tstr_t *haystack, tstr_t *needle)
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

    ret = s;
    if (TSTR_IS_ALLOCATED(&s))
    {
	TPTR(&ret) = tmalloc(ret.len, "dupped str");
	memcpy(TPTR(&ret), TPTR(&s), ret.len);
    }
    return ret;
}

tstr_t tstr_slice(tstr_t s, int index, int count)
{
    tstr_t ret;

    ret = s;
    TPTR(&ret) += index;
    ret.len = count;
    return tstr_dup(ret);
}

void tstr_free(tstr_t *s)
{
    if (TSTR_IS_ALLOCATED(s))
	tfree(TPTR(s));
}

void tstr_cat(tstr_t *dst, tstr_t *a, tstr_t *b)
{
    dst->len = a->len + b->len;
    TPTR(dst) = tmalloc(dst->len, "string");
    memcpy(TPTR(dst), TPTR(a), a->len);
    memcpy(TPTR(dst) + a->len, TPTR(b), b->len);
    TSTR_SET_ALLOCATED(dst);
}

void tstr_unescape(tstr_t *dst, tstr_t *src)
{
    unsigned short left;
    char *in, *out;

    out = TPTR(dst) = tmalloc(src->len, "Unescaped string");

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
	    break;
	}
    }
    dst->len = out - TPTR(dst);
    TSTR_SET_ALLOCATED(dst);
}

/* XXX: this is stupid. Should use tstr and have tstr prefix comp */
int prefix_comp(int len, char *a, char *b)
{
    int i;

    for (i = 0; i < len; i++)
    {
	if (a[i] != b[i])
	    return -1;
    }
    return 0;
}

tstr_t tstr_cut(tstr_t *t, char delim)
{
    int i;
    tstr_t ret;

    ret = *t;
    for (i = 0; i < t->len && TPTR(t)[i] != delim; i++);
    if (i == t->len)
    {
	TPTR(t) = NULL;
	t->len = 0;
	return ret;
    }

    ret.len = i; 
    /* Skip over prefix + delim */
    tstr_advance(t, i + 1);
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
