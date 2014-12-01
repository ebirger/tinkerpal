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
#include <string.h>
#include <ctype.h>
#include "util/tnum.h"
#include "util/tprintf.h"
#include "util/debug.h"

static inline int is_oct_digit(char c)
{
    return c >= '0' && c <= '7';
}

static inline int is_bin_digit(char c)
{
    return c >= '0' && c <= '1';
}

static int is_fp(const tstr_t *s)
{
    int i;

    for (i = 0; i < s->len; i++)
    {
        if (tstr_peek(s, i) == '.')
            return 1;
    }
    return 0;
}

static inline int is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int tstr_to_tnum(tnum_t *ret, const tstr_t *s)
{
    int i = 0, fp = 0, e = 0, exp = 0, radix = 10, sign = 1;
    tnum_t v = {};

    /* Skip leading whitespace
     * XXX: skip trailing whitespace too */
    /* XXX: should be using tstr iterator API */
    while (i < s->len && is_whitespace(tstr_peek(s, i)))
        i++;

    if (i == s->len)
        goto Exit;

    if (tstr_peek(s, i) == '-')
    {
        sign = -1;
        i++;
    }

    if (s->len - i > 1 && tstr_peek(s, i) == '0' && !is_fp(s))
    {
        radix = 8;
        i++;
        if (s->len - i > 1)
        {
            switch (tstr_peek(s,i))
            {
            case 'b':
            case 'B':
                radix = 2;
                i++;
                break;
            case 'o':
            case 'O':
                radix = 8;
                i++;
                break;
            case 'x':
            case 'X':
                radix = 16;
                i++;
                break;
            }
        }
    }

    for (; i < s->len; i++)
    {
        char c = tstr_peek(s, i);

        if ((radix == 16 && !isxdigit((int)c)) ||
            (radix == 10 && (!isdigit((int)c) && c != '.' && c != 'e')) ||
            (radix == 8 && !is_oct_digit(c)) ||
            (radix == 2 && !is_bin_digit(c)))
        {
            return -1;
        }

        if (c == '.')
        {
            if (fp || e)
                return -1;

            fp = i+1;
            v.value.fp = v.value.i;
            v.flags = NUMERIC_FLAG_FP;
            continue;
        }

        if (c == 'e' && radix != 16)
        {
            if (e)
                return -1;

            e = i+1;
            continue;
        }

        if (e)
        {
            exp = exp * 10 + (c - '0');
            continue;
        }

        if (fp)
        {
            double addition = (double)digit_value(c)/exp_power(i + 1 - fp);
            v.value.fp = v.value.fp + addition;
            continue;
        }
        v.value.i = v.value.i * radix + digit_value(c);
    }
    if (e == s->len || fp == s->len)
        return -1;

    if (fp)
        v.value.fp *= (exp_power(exp) * sign);
    else
        v.value.i *= (exp_power(exp) * sign);

Exit:
    *ret = v;
    return 0;
}

tstr_t _int_to_tstr(int i, int base)
{
    static char buf[32];
    tstr_t ret;

    tsn_itoa(buf, sizeof(buf), i, base);
    tstr_cpy_str(&ret, buf);
    return ret;
}

tstr_t double_to_tstr(double d)
{
    static char buf[32];
    tstr_t ret;

    tsnprintf(buf, sizeof(buf), "%f", d);
    tstr_cpy_str(&ret, buf);
    return ret;
}
