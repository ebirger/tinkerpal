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
#ifndef __JS_SCAN_H__
#define __JS_SCAN_H__

#include "util/tstr.h"
#include "util/tnum.h"
 /* XXX: the following should be removed once js_scan_match is removed */
#include "util/debug.h"

#define EQ (1<<8)
#define DOUBLE (1<<9)
#define TRIPPLE (1<<10)
#define STRICT (1<<11)

#define TOK_NONE 0
#define TOK_ID 1
#define TOK_NUM 2
#define TOK_EOF 3
#define TOK_IF 4
#define TOK_WHILE 5
#define TOK_ELSE 6
#define TOK_FUNCTION 7
#define TOK_RETURN 8
#define TOK_CONTINUE 9
#define TOK_BREAK 10
#define TOK_VAR 11
#define TOK_STRING 12
#define TOK_THIS 13
#define TOK_PROTOTYPE 14
#define TOK_NEW 15
#define TOK_FOR 16
#define TOK_TRUE 17
#define TOK_FALSE 18
#define TOK_NULL 19
#define TOK_UNDEFINED 20
#define TOK_THROW 21
#define TOK_TRY 22
#define TOK_CATCH 23
#define TOK_CONSTANT 24
#define TOK_SWITCH 25
#define TOK_CASE 26
#define TOK_DEFAULT 27
#define TOK_IN 29
#define TOK_DO 30
#define TOK_DOT '.'
#define TOK_COMMA ','
#define TOK_COLON ':'
#define TOK_OPEN_MEMBER '['
#define TOK_CLOSE_MEMBER ']'
#define TOK_QUESTION '?'
#define TOK_END_STATEMENT ';'
#define TOK_OPEN_SCOPE '{'
#define TOK_CLOSE_SCOPE '}'
#define TOK_OPEN_PAREN '('
#define TOK_CLOSE_PAREN ')'
#define TOK_EQ '='
#define TOK_IS_EQ ('=' | DOUBLE)
#define TOK_IS_EQ_STRICT ('=' | DOUBLE | STRICT)
#define TOK_NOT '!'
#define TOK_NOT_EQ ('!' | EQ)
#define TOK_NOT_EQ_STRICT ('!' | EQ | STRICT)
#define TOK_TILDE '~'
#define TOK_PLUS '+'
#define TOK_PLUS_PLUS ('+' | DOUBLE)
#define TOK_PLUS_EQ ('+' | EQ)
#define TOK_MINUS '-'
#define TOK_MINUS_MINUS ('-' | DOUBLE)
#define TOK_MINUS_EQ ('-' | EQ)
#define TOK_MULT '*'
#define TOK_MULT_EQ ('*' | EQ)
#define TOK_DIV '/'
#define TOK_DIV_EQ ('/' | EQ)
#define TOK_AND '&'
#define TOK_LOG_AND ('&' | DOUBLE)
#define TOK_AND_EQ ('&' | EQ)
#define TOK_OR '|'
#define TOK_LOG_OR ('|' | DOUBLE)
#define TOK_OR_EQ ('|' | EQ)
#define TOK_XOR '^'
#define TOK_XOR_EQ ('^' | EQ)
#define TOK_MOD '%'
#define TOK_MOD_EQ ('%' | EQ)
#define TOK_GR '>'
#define TOK_GE ('>' | EQ)
#define TOK_LT '<'
#define TOK_LE ('<' | EQ)
#define TOK_SHR ('>' | DOUBLE)
#define TOK_SHR_EQ ('>' | DOUBLE | EQ)
#define TOK_SHRZ ('>' | DOUBLE | TRIPPLE)
#define TOK_SHRZ_EQ ('>' | DOUBLE | TRIPPLE | EQ)
#define TOK_SHL ('<' | DOUBLE)
#define TOK_SHL_EQ ('<' | DOUBLE | EQ)

typedef int token_type_t;
typedef struct scan_t scan_t;
typedef enum {
    TOKEN_GRP_NONE = 0,
    TOKEN_GRP_CONTROL = 1,
    TOKEN_GRP_DATA = 2,
    TOKEN_GRP_SCOPE = 3,
    TOKEN_GRP_CONSTANT = 4,
} token_group_t;

 /* trick - we would like to avoid function calls in order to get current
  * token, so we assume scan_t's first member is token_type_t 
  */
#define CUR_TOK(s) (*((token_type_t *)(s)))

int js_scan_get_identifier(scan_t *scan, tstr_t *id);
int js_scan_get_string(scan_t *scan, tstr_t *str);
int js_scan_get_num(scan_t *scan, tnum_t *ret);
int js_scan_get_constant(scan_t *scan);

void js_scan_set_trace_point(scan_t *scan);
void js_scan_trace(scan_t *scan);

/* _js_scan_match does not panic on error */
int _js_scan_match(scan_t *scan, token_type_t tok);

/* js_scan_match panics on error.
 * Ideally, this API would be removed once all such assertions are
 * converted to exceptions
 */
static inline void js_scan_match(scan_t *scan, token_type_t tok)
{
    if (_js_scan_match(scan, tok))
    {
	tp_crit(("expected %x:%c, got %x:%c\n", tok, tok, CUR_TOK(scan), 
            CUR_TOK(scan)));
    }
}

void js_scan_next_token(scan_t *scan);

int js_scan_get_remaining(scan_t *scan);

token_group_t js_scan_get_token_group(scan_t *scan);

scan_t *js_scan_save(scan_t *scan);
void js_scan_restore(scan_t *dst, scan_t *src);
scan_t *js_scan_slice(scan_t *start, scan_t *end);
void js_scan_free(scan_t *scan);

void js_scan_uninit(scan_t *scan);
scan_t *_js_scan_init(tstr_t *data, int own_data);
static inline scan_t *js_scan_init(tstr_t *data)
{
    return _js_scan_init(data, 0);
}

/* Global initialization function, not tied to a scan_t instance.
 * cb returns 0 if s matches a known constant, the constant's value
 * is returned in *constant
 */
typedef int (*get_constants_cb_t)(int *constant, tstr_t *s);
void js_scan_set_constants_cb(get_constants_cb_t cb);

#endif
