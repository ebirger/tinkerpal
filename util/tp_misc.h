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
#ifndef __TP_MISC_H__
#define __TP_MISC_H__

#include <stddef.h>
#include "util/tp_types.h"

#define bit_set(x, bit, val) x = (val) ? x | (bit) : x & ~(bit)
#define bit_get(x, bit) (!!((x) & (bit)))

#define container_of(ptr, type, member) \
    ((type *)((u8 *)(ptr) - offsetof(type, member)))

#define ARRAY_SIZE(a) ((sizeof(a)) / sizeof((a)[0]))

#define MIN(a, b) ((a) > (b) ? (b) : (a))

/* Returns 10^exp */
static inline u64 exp_power(int exp)
{
    u64 pow;

    switch (exp)
    {
    case 0: pow = 1; break;
    case 1: pow = 10; break;
    case 2: pow = 100; break;
    case 3: pow = 1000; break;
    case 4: pow = 10000; break;
    case 5: pow = 100000; break;
    case 6: pow = 1000000; break;
    default:
        pow = 1;
        while (exp--)
            pow *= 10;
    }
    return pow;
}

/* The PP_NARG macro returns the number of arguments that have been
  * passed to it.
  * found at: https://groups.google.com/forum/#!topic/comp.std.c/d-6Mj5Lko_s
  */
#define PP_NARG(...) PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10, \
    _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
    _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
    _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
    _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
    _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
    _61,_62,_63,N,...) N
#define PP_RSEQ_N() 63,62,61,60, \
    59,58,57,56,55,54,53,52,51,50, \
    49,48,47,46,45,44,43,42,41,40, \
    39,38,37,36,35,34,33,32,31,30, \
    29,28,27,26,25,24,23,22,21,20, \
    19,18,17,16,15,14,13,12,11,10, \
    9,8,7,6,5,4,3,2,1,0

/* CHAR_TO_WCHAR: Macro insanity :) convert a list of chars
 * (e.g. ('a', 'b', 'c')) to a list of wchars (e.g. 'a', 0, 'b', 0, 'c', 0)
 */
#define CHAR_TO_WCHAR(args...) _CHAR_TO_WCHAR(PP_NARG(args),args)
#define __CHAR_TO_WCHAR(n, args...) CHAR_TO_WCHAR##n(args)
#define _CHAR_TO_WCHAR(n, args...) __CHAR_TO_WCHAR(n, args)
#define CHAR_TO_WCHAR1(c) c, 0
#define CHAR_TO_WCHAR2(c, args...) c, 0, CHAR_TO_WCHAR1(args)
#define CHAR_TO_WCHAR3(c, args...) c, 0, CHAR_TO_WCHAR2(args)
#define CHAR_TO_WCHAR4(c, args...) c, 0, CHAR_TO_WCHAR3(args)
#define CHAR_TO_WCHAR5(c, args...) c, 0, CHAR_TO_WCHAR4(args)
#define CHAR_TO_WCHAR6(c, args...) c, 0, CHAR_TO_WCHAR5(args)
#define CHAR_TO_WCHAR7(c, args...) c, 0, CHAR_TO_WCHAR6(args)
#define CHAR_TO_WCHAR8(c, args...) c, 0, CHAR_TO_WCHAR7(args)
#define CHAR_TO_WCHAR9(c, args...) c, 0, CHAR_TO_WCHAR8(args)
#define CHAR_TO_WCHAR10(c, args...) c, 0, CHAR_TO_WCHAR9(args)
#define CHAR_TO_WCHAR11(c, args...) c, 0, CHAR_TO_WCHAR10(args)
#define CHAR_TO_WCHAR12(c, args...) c, 0, CHAR_TO_WCHAR11(args)
#define CHAR_TO_WCHAR13(c, args...) c, 0, CHAR_TO_WCHAR12(args)
#define CHAR_TO_WCHAR14(c, args...) c, 0, CHAR_TO_WCHAR13(args)
#define CHAR_TO_WCHAR15(c, args...) c, 0, CHAR_TO_WCHAR14(args)
#define CHAR_TO_WCHAR16(c, args...) c, 0, CHAR_TO_WCHAR15(args)
#define CHAR_TO_WCHAR17(c, args...) c, 0, CHAR_TO_WCHAR16(args)
#define CHAR_TO_WCHAR18(c, args...) c, 0, CHAR_TO_WCHAR17(args)
#define CHAR_TO_WCHAR19(c, args...) c, 0, CHAR_TO_WCHAR18(args)
#define CHAR_TO_WCHAR20(c, args...) c, 0, CHAR_TO_WCHAR19(args)
#define CHAR_TO_WCHAR21(c, args...) c, 0, CHAR_TO_WCHAR20(args)
#define CHAR_TO_WCHAR22(c, args...) c, 0, CHAR_TO_WCHAR21(args)
#define CHAR_TO_WCHAR23(c, args...) c, 0, CHAR_TO_WCHAR22(args)
#define CHAR_TO_WCHAR24(c, args...) c, 0, CHAR_TO_WCHAR23(args)
#define CHAR_TO_WCHAR25(c, args...) c, 0, CHAR_TO_WCHAR24(args)
#define CHAR_TO_WCHAR26(c, args...) c, 0, CHAR_TO_WCHAR25(args)
#define CHAR_TO_WCHAR27(c, args...) c, 0, CHAR_TO_WCHAR26(args)
#define CHAR_TO_WCHAR28(c, args...) c, 0, CHAR_TO_WCHAR27(args)
#define CHAR_TO_WCHAR29(c, args...) c, 0, CHAR_TO_WCHAR28(args)
#define CHAR_TO_WCHAR30(c, args...) c, 0, CHAR_TO_WCHAR29(args)
#define CHAR_TO_WCHAR31(c, args...) c, 0, CHAR_TO_WCHAR30(args)
#define CHAR_TO_WCHAR32(c, args...) c, 0, CHAR_TO_WCHAR31(args)
#define CHAR_TO_WCHAR33(c, args...) c, 0, CHAR_TO_WCHAR32(args)
#define CHAR_TO_WCHAR34(c, args...) c, 0, CHAR_TO_WCHAR33(args)
#define CHAR_TO_WCHAR35(c, args...) c, 0, CHAR_TO_WCHAR34(args)
#define CHAR_TO_WCHAR36(c, args...) c, 0, CHAR_TO_WCHAR35(args)
#define CHAR_TO_WCHAR37(c, args...) c, 0, CHAR_TO_WCHAR36(args)
#define CHAR_TO_WCHAR38(c, args...) c, 0, CHAR_TO_WCHAR37(args)
#define CHAR_TO_WCHAR39(c, args...) c, 0, CHAR_TO_WCHAR38(args)
#define CHAR_TO_WCHAR40(c, args...) c, 0, CHAR_TO_WCHAR39(args)
#define CHAR_TO_WCHAR41(c, args...) c, 0, CHAR_TO_WCHAR40(args)
#define CHAR_TO_WCHAR42(c, args...) c, 0, CHAR_TO_WCHAR41(args)
#define CHAR_TO_WCHAR43(c, args...) c, 0, CHAR_TO_WCHAR42(args)
#define CHAR_TO_WCHAR44(c, args...) c, 0, CHAR_TO_WCHAR43(args)
#define CHAR_TO_WCHAR45(c, args...) c, 0, CHAR_TO_WCHAR44(args)
#define CHAR_TO_WCHAR46(c, args...) c, 0, CHAR_TO_WCHAR45(args)
#define CHAR_TO_WCHAR47(c, args...) c, 0, CHAR_TO_WCHAR46(args)
#define CHAR_TO_WCHAR48(c, args...) c, 0, CHAR_TO_WCHAR47(args)
#define CHAR_TO_WCHAR49(c, args...) c, 0, CHAR_TO_WCHAR48(args)
#define CHAR_TO_WCHAR50(c, args...) c, 0, CHAR_TO_WCHAR49(args)
#define CHAR_TO_WCHAR51(c, args...) c, 0, CHAR_TO_WCHAR50(args)
#define CHAR_TO_WCHAR52(c, args...) c, 0, CHAR_TO_WCHAR51(args)
#define CHAR_TO_WCHAR53(c, args...) c, 0, CHAR_TO_WCHAR52(args)
#define CHAR_TO_WCHAR54(c, args...) c, 0, CHAR_TO_WCHAR53(args)
#define CHAR_TO_WCHAR55(c, args...) c, 0, CHAR_TO_WCHAR54(args)
#define CHAR_TO_WCHAR56(c, args...) c, 0, CHAR_TO_WCHAR55(args)
#define CHAR_TO_WCHAR57(c, args...) c, 0, CHAR_TO_WCHAR56(args)
#define CHAR_TO_WCHAR58(c, args...) c, 0, CHAR_TO_WCHAR57(args)
#define CHAR_TO_WCHAR59(c, args...) c, 0, CHAR_TO_WCHAR58(args)
#define CHAR_TO_WCHAR60(c, args...) c, 0, CHAR_TO_WCHAR49(args)
#define CHAR_TO_WCHAR61(c, args...) c, 0, CHAR_TO_WCHAR59(args)
#define CHAR_TO_WCHAR62(c, args...) c, 0, CHAR_TO_WCHAR61(args)
#define CHAR_TO_WCHAR63(c, args...) c, 0, CHAR_TO_WCHAR62(args)

#endif
