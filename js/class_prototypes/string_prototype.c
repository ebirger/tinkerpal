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
#include "js/js_obj.h"

int do_string_prototype_split(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    tstr_t cur, sep;
    int idx = 0;

    tp_assert(argc == 1); /* XXX: support limit */

    *ret = array_new();

    sep = obj_get_str(argv[0]);
    cur = obj_get_str(this);
    while (1)
    {
	tstr_t n = cur;

	idx = tstr_find(&cur, &sep);
	if (idx == -1)
	    break;

	n.len = idx;
	array_push(*ret, string_new(tstr_dup(n)));
	idx += sep.len;
	cur.len -= idx;
	cur.value += idx;
    }
    if (cur.len)
	array_push(*ret, string_new(tstr_dup(cur)));
    tstr_free(&sep);
    tstr_free(&cur);
    return 0;
}

int do_string_prototype_indexof(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    tstr_t needle, haystack;
    int idx;

    tp_assert(argc == 1); /* XXX: support position */

    haystack = obj_get_str(this);
    needle = obj_get_str(argv[0]);

    idx = tstr_find(&haystack, &needle);

    tstr_free(&haystack);
    tstr_free(&needle);
    *ret = num_new_int(idx);
    return 0;
}

int do_string_prototype_substring(obj_t **ret, function_t *func, 
    obj_t *this, int argc, obj_t *argv[])
{
    int start, end;
    tstr_t s, retval;

    tp_assert(argc == 1 || argc == 2);

    retval = s = obj_get_str(this);

    start = obj_get_int(argv[0]);
    end = argc == 2 ? obj_get_int(argv[1]) : s.len;

    /* We don't allow bad params here :) */
    tp_assert(start >= 0 && start < s.len);
    tp_assert(end > start && end <= s.len);

    retval.value += start;
    retval.len = end - start;
    *ret = string_new(tstr_dup(retval));
    tstr_free(&s);
    return 0;
}

int do_string_prototype_char_at(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    int pos;
    tstr_t s, retval = S("");

    s = obj_get_str(this);

    pos = !argc ? -1 : obj_get_int(argv[0]);

    if (pos < 0 || pos >= s.len)
	goto Exit;

    retval = s;
    retval.value += pos;
    retval.len = 1;

Exit:
    *ret = string_new(tstr_dup(retval));
    tstr_free(&s);
    return 0;
}

int do_string_prototype_char_code_at(obj_t **ret, function_t *func, 
    obj_t *this, int argc, obj_t *argv[])
{
    int pos;
    tstr_t s;

    *ret = NAN_OBJ;
    s = obj_get_str(this);

    pos = !argc ? -1 : obj_get_int(argv[0]);

    if (pos < 0 || pos >= s.len)
	goto Exit;

    *ret = num_new_int(*(s.value + pos));

Exit:
    tstr_free(&s);
    return 0;
}

int do_string_constructor(obj_t **ret, function_t *func, 
    obj_t *this_obj, int argc, obj_t *argv[])
{
    tstr_t input = S("");

    if (argc == 1)
	input = obj_get_str(argv[0]);

    *ret = string_new(tstr_dup(input));
    tstr_free(&input);
    return 0;
}
