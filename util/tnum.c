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
#include "util/tmalloc.h"
#include "util/tnum.h"
#include "util/debug.h"

static int is_oct_digit(char c)
{
    return c >= '0' && c <= '7';
}

int exp_power(int exp)
{
    int pow = 1;

    if (!exp)
	return 1;

    while (exp--)
	pow *= 10;
    return pow;
}

static int is_fp(const tstr_t *s)
{
    int i;

    for (i = 0; i < s->len; i++)
    {
	if (s->value[i] == '.')
	    return 1;
    }
    return 0;
}

int tstr_to_tnum(tnum_t *ret, const tstr_t *s)
{
    int i = 0, fp = 0, e = 0, exp = 0, radix = 10, sign = 1;
    tnum_t v = {};

    if (s->value[i] == '-')
    {
	sign = -1;
	i++;
    }

    if (s->len > 2 && s->value[i] == '0' && !is_fp(s))
    {
	radix = 8;
	i++;
	if (s->len > 2 && s->value[1] == 'x')
	{
	    radix = 16;
	    i++;
	}
    }

    for (; i < s->len; i++)
    {
	char c = s->value[i];

	if ((radix == 16 && !isxdigit((int)c)) ||
	    (radix == 10 && (!isdigit((int)c) && c != '.' && c != 'e')) ||
	    (radix == 8 && !is_oct_digit(c)))
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

    *ret = v;
    return 0;
}

tstr_t tnum_to_tstr(tnum_t v)
{
    static char buf[32];
    tstr_t ret;

    if (NUMERIC_IS_FP(v))
	ret.len = snprintf(buf, sizeof(buf), "%f", NUMERIC_FP(v));
    else
	ret.len = snprintf(buf, sizeof(buf), "%d", NUMERIC_INT(v));
    ret.value = tmalloc(ret.len, "tnum str");
    memcpy(ret.value, buf, ret.len);
    TSTR_SET_ALLOCATED(&ret);
    return ret;
}

tstr_t int_to_tstr(int i)
{
    tnum_t n = {};

    NUMERIC_INT(n) = i;
    return tnum_to_tstr(n);
}

tstr_t double_to_tstr(double d)
{
    tnum_t n = {};

    n.flags = NUMERIC_FLAG_FP;
    NUMERIC_FP(n) = d;
    return tnum_to_tstr(n);
}
