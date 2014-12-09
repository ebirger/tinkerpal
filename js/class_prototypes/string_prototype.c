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
#include "js/js_utils.h"

int do_string_prototype_split(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    tstr_t orig, cur, sep;

    /* XXX: support limit */
    *ret = array_new();
    orig = cur = obj_get_str(this);

    if (argc == 1 || argv[1] == UNDEF)
        goto Exit;

    sep = obj_get_str(argv[1]);
    while (1)
    {
        tstr_t a, b;
        int idx;

        idx = tstr_find(&cur, &sep);
        if (idx == -1)
            break;

        tstr_split(&cur, &a, &b, idx, sep.len);
        array_push(*ret, string_new(tstr_dup(a)));
        cur = b;
    }
    tstr_free(&sep);

Exit:
    array_push(*ret, string_new(tstr_dup(cur)));
    tstr_free(&orig);
    return 0;
}

int do_string_prototype_indexof(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    tstr_t needle, haystack;
    int idx;

    /* XXX: support position */
    if (argc == 1)
    {
        *ret = num_new_int(-1);
        return 0;
    }

    haystack = obj_get_str(this);
    needle = obj_get_str(argv[1]);

    idx = tstr_find(&haystack, &needle);

    tstr_free(&haystack);
    tstr_free(&needle);
    *ret = num_new_int(idx);
    return 0;
}

int do_string_prototype_substring(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    int start, end;
    tstr_t s, retval;

    if (argc != 2 && argc != 3)
        return js_invalid_args(ret);

    s = obj_get_str(this);

    start = obj_get_int(argv[1]);
    end = argc == 3 ? obj_get_int(argv[2]) : s.len;

    /* We don't allow bad params here :) */
    if (start < 0 || start >= s.len || end <= start || end > s.len)
        return js_invalid_args(ret);

    retval = tstr_slice(&s, start, end - start);
    *ret = string_new(retval);
    tstr_free(&s);
    return 0;
}

int do_string_prototype_char_at(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    int pos;
    tstr_t s, retval = S("");

    s = obj_get_str(this);

    pos = argc == 1 ? -1 : obj_get_int(argv[1]);

    if (pos < 0 || pos >= s.len)
        goto Exit;

    retval = tstr_slice(&s, pos, 1);

Exit:
    *ret = string_new(retval);
    tstr_free(&s);
    return 0;
}

int do_string_prototype_char_code_at(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    int pos;
    tstr_t s;

    *ret = NAN_OBJ;
    s = obj_get_str(this);

    pos = argc == 1 ? -1 : obj_get_int(argv[1]);

    if (pos < 0 || pos >= s.len)
        goto Exit;

    *ret = num_new_int(tstr_peek(&s, pos));

Exit:
    tstr_free(&s);
    return 0;
}

static int string_to_upper_lower_case(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[], int is_lower)
{
    tstr_t s;

    s = obj_get_str(this);

    *ret = string_new(tstr_to_upper_lower(s, is_lower));

    tstr_free(&s);
    return 0;
}

int do_string_prototype_to_lower_case(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    return string_to_upper_lower_case(ret, this, argc, argv, 1);
}

int do_string_prototype_to_upper_case(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    return string_to_upper_lower_case(ret, this, argc, argv, 0);
}

int do_string_constructor(obj_t **ret, obj_t *this_obj, int argc, obj_t *argv[])
{
    tstr_t input = S("");

    if (argc == 2)
        input = obj_get_str(argv[1]);

    *ret = string_new(tstr_dup(input));
    tstr_free(&input);
    return 0;
}
