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
#include "js/js_obj.h"
#include "js/js_utils.h"
#include <math.h>
#include <stdlib.h> /* abs */

#define MATH_FUNC1(type, name) \
int do_##name(obj_t **ret, obj_t *this, int argc, obj_t *argv[]) \
{ \
    if (argc != 2) \
    	return js_invalid_args(ret); \
    *ret = num_new_##type(name(obj_get_##type(argv[1]))); \
    return 0; \
}
#define MATH_FUNC2(type, name) \
int do_##name(obj_t **ret, obj_t *this, int argc, obj_t *argv[]) \
{ \
    if (argc != 3) \
    	return js_invalid_args(ret); \
    *ret = num_new_##type(name(obj_get_##type(argv[1]), obj_get_##type(argv[2]))); \
    return 0; \
}

MATH_FUNC1(fp, sin)
MATH_FUNC1(fp, asin)
MATH_FUNC1(fp, cos)
MATH_FUNC1(fp, acos)
MATH_FUNC1(fp, tan)
MATH_FUNC1(fp, atan)
MATH_FUNC1(fp, sqrt)
MATH_FUNC1(fp, log)
MATH_FUNC1(fp, exp)
MATH_FUNC1(fp, floor)
MATH_FUNC1(fp, ceil)
MATH_FUNC1(fp, round)
MATH_FUNC1(int, abs)
MATH_FUNC2(fp, atan2)
MATH_FUNC2(fp, pow)

#undef MATH_FUNC1
#undef MATH_FUNC2
