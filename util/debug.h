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
#ifndef __TP_DEBUG_H__
#define __TP_DEBUG_H__

#define L_DEBUG 4
#define L_INFO 3
#define L_WARN 2
#define L_ERR 1
#define L_CRIT 0

#ifndef LOG_LEVEL
#define LOG_LEVEL L_WARN
#endif

typedef struct {
    void (*print)(char *fmt, ...);
    int (*write)(char *buf, int len);
    void (*panic)(void);
} debugfn_t;

extern debugfn_t debugfn;

void debug_init(debugfn_t *fn);

#define tp_out_bin(buf, len) debugfn.write(buf, len)
#define tp_out(x) debugfn.print x

#define tp_log(level, x) do { \
    tp_out(("%s: ", level)); \
    tp_out(x); \
} while(0)

#define tp_crit(x) do { tp_log("CRIT", x); debugfn.panic(); } while(0)

#if LOG_LEVEL >= L_ERR
#define tp_err(x) tp_log("ERROR", x)
#else
#define tp_err(x)
#endif
#if LOG_LEVEL >= L_WARN
#define tp_warn(x) tp_log("WARN", x)
#else
#define tp_warn(x)
#endif
#if LOG_LEVEL >= L_INFO
#define tp_info(x) tp_log("INFO", x)
#else
#define tp_info(x)
#endif
#if LOG_LEVEL >= L_DEBUG
#define tp_debug(x) tp_log("DEBUG", x)
#else
#define tp_debug(x)
#endif

#define tp_assert(x) do { \
    if (!(x)) \
        tp_crit(("%s: assertion %s failed\n", __FUNCTION__, #x)); \
} while (0)

void hexdump(const unsigned char *buf, int len);

#endif
