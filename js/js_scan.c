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
#include "util/tnum.h"
#include "util/debug.h"
#include "mem/tmalloc.h"
#include "js/js_scan.h"

typedef union {
    tstr_t identifier;
    tstr_t string;
    tnum_t num;
    int constant;
} scan_value_t;

struct scan_t {
    token_type_t tok; /* Must be first */
    tstr_t code;
    int lpc;
    int pc;
    int trace_point;
    int last_token_start;
    tstr_t *internal_buf;
    int size; /* should be size_t */
    char look;
#define SCAN_FLAG_EOF 0x0001
#define SCAN_FLAG_INVALID 0x0002
    unsigned short flags;
    scan_value_t value;
};

static get_constants_cb_t g_get_constants_cb;

#define IS_EOF(scan) ((scan)->flags & SCAN_FLAG_EOF)
#define SET_EOF(scan) ((scan)->flags |= SCAN_FLAG_EOF)

static inline int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static inline int is_x_digit(char c)
{
    return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static inline int is_number_letter(char c)
{
    return is_x_digit((int)c) || c == '.' || c == 'e' || c == 'x' ||
        c == 'X' || c == 'b' || c == 'B' || c == 'o' || c == 'O';
}

static inline int is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline int is_valid_identifier_first_letter(char c)
{
    return is_alpha((int)c) || c == '_' || c == '$';
}

static inline int is_valid_identifier_non_first_letter(char c)
{
    return is_valid_identifier_first_letter(c) || is_digit(c);
}

static inline int is_newline(char c)
{
    return c == '\n' || c == '\r';
}

static inline int is_whitespace(char c)
{
    return c == ' ' || c == '\t' || is_newline(c);
}

/* Horrible name - not correct, just means we convert to token directly with no
 * special care */
static inline int is_control_char(char c)
{
    return c == '{' || c == '}' || c == '(' || c == ')' || c == ';' || 
        c == ',' || c == '?' || c == ':' || c == '.' || c == '[' || c == ']';
}

static inline int is_string_delim(char c)
{
    return c == '\'' || c == '"';
}

static void _get_char(scan_t *scan)
{
    if (scan->size > 0)
    {
        scan->lpc = scan->pc;
        if (scan->size > 1)
            scan->look = tstr_peek(&scan->code, scan->pc);
        scan->pc++;
        scan->size--;
    }
    if (scan->size == 0)
    {
        scan->look = 255; /* invalid char */
        SET_EOF(scan);
        return;
    }
    tp_debug("[%x:%c] ", scan->look, scan->look);
}

static void skip_white(scan_t *scan)
{
    while (1)
    {
        char next;

        while (is_whitespace(scan->look) && !IS_EOF(scan))
            _get_char(scan);

        if (scan->look != '/')
            break;
        
        /* XXX: rewrite this pretty */
	next = tstr_peek(&scan->code, scan->pc);
        /* Skip single line comment */
        if (next == '/')
        {
            _get_char(scan); /* Skip '/' */
            _get_char(scan); /* Skip '/' */
            while (!is_newline(scan->look) && !IS_EOF(scan))
                _get_char(scan);
        }
        /* Skip multiline comments */
        else if (next == '*')
        {
            _get_char(scan); /* Skip '/' */
            _get_char(scan); /* Skip '*' */
            /* XXX: assert on nested comments */
            while (scan->look != '*' &&
                tstr_peek(&scan->code, scan->pc) != '/' && !IS_EOF(scan))
	    {
                _get_char(scan);
	    }
            _get_char(scan); /* Skip '*' */
            _get_char(scan); /* Skip '/' */
        }
        else
            break;
    }
}

static inline tstr_t extract_string(scan_t *scan)
{
    tstr_t ret;
    unsigned short tflags = 0;
    int start;
    token_type_t delim = scan->look;

    _get_char(scan); /* skip enclosure */
    start = scan->lpc;
    while (scan->look != delim && !IS_EOF(scan))
    {
        if (is_newline(scan->look))
            tp_crit("Newlines are not allowed in strings\n");

        if (scan->look == '\\')
        {
            tflags |= TSTR_FLAG_ESCAPED;
            _get_char(scan);
        }

        _get_char(scan);
    }

    ret = tstr_piece(&scan->code, start, scan->lpc - start);
    ret.flags |= tflags;
    _get_char(scan); /* skip enclosure */
    skip_white(scan);
    return ret;
}

static inline tstr_t extract_identifier(scan_t *scan)
{
    tstr_t ret;
    unsigned short tflags = 0;
    int start;

    start = scan->lpc;
    while (is_valid_identifier_non_first_letter(scan->look))
        _get_char(scan);

    ret = tstr_piece(&scan->code, start, scan->lpc - start);
    ret.flags |= tflags;
    skip_white(scan);
    return ret;
}

/* Hash table for keywords based on length.
 * GCC offers an extremely nice way to do this using anonymous members 
 * but TI CCS5 doesn't support it. sigh...
 */
typedef struct {
    const char *str;
    const token_type_t tok;
} keyword_t;

#define K(n, o) { .str = n, .tok = o }

static const keyword_t keywords2[] = {
    K("if", TOK_IF), 
    K("in", TOK_IN), 
    K("do", TOK_DO), 
    {} 
};

static const keyword_t keywords3[] = {
    K("for", TOK_FOR), 
    K("var", TOK_VAR), 
    K("new", TOK_NEW), 
    K("try", TOK_TRY), 
    {} 
};

static const keyword_t keywords4[] = {
    K("else", TOK_ELSE), 
    K("this", TOK_THIS), 
    K("true", TOK_TRUE), 
    K("null", TOK_NULL), 
    K("case", TOK_CASE), 
    {} 
};

static const keyword_t keywords5[] = {
    K("while", TOK_WHILE),
    K("break", TOK_BREAK),
    K("false", TOK_FALSE),
    K("throw", TOK_THROW),
    K("catch", TOK_CATCH),
    {}
};

static const keyword_t keywords6[] = {
    K("switch", TOK_SWITCH),
    K("return", TOK_RETURN),
    {}
};

static const keyword_t keywords7[] = {
    K("default", TOK_DEFAULT),
    {}
};

static const keyword_t keywords8[] = {
    K("function", TOK_FUNCTION),
    K("continue", TOK_CONTINUE),
    {}
};

static const keyword_t keywords9[] = {
    K("prototype", TOK_PROTOTYPE),
    K("undefined", TOK_UNDEFINED),
    K("arguments", TOK_ARGUMENTS),
    {}
};

static const keyword_t *keywords[] = {
    [2] = keywords2,
    [3] = keywords3,
    [4] = keywords4,
    [5] = keywords5,
    [6] = keywords6,
    [7] = keywords7,
    [8] = keywords8,
    [9] = keywords9,
};

static inline token_type_t identifier_to_tok(const tstr_t *str)
{
    const keyword_t *k;
    int len = str->len;

    if (len < 2 || len > 9)
        return TOK_ID;

    for (k = keywords[len]; k->tok; k++)
    {
        /* Exit on exact match */
        if (!_tstr_cmp_str(str, k->str, len))
            return k->tok;
    }

    return TOK_ID;
}

static inline tnum_t extract_num(scan_t *scan)
{
    tstr_t s;
    tnum_t ret;
    int start;

    start = scan->lpc;
    while (is_number_letter(scan->look))
        _get_char(scan);
    s = tstr_piece(&scan->code, start, scan->lpc - start);
    skip_white(scan);

    if (tstr_to_tnum(&ret, &s))
    {
        /* Invalid number, mark scan as invalid */
        scan->flags |= SCAN_FLAG_INVALID;
    }

    return ret;
}

void js_scan_next_token(scan_t *scan)
{
    char next = 0, next2 = 0, next3 = 0;

    scan->tok = 0;
    scan->flags &= ~SCAN_FLAG_INVALID;
    scan->last_token_start = scan->lpc;
    if (is_control_char(scan->look))
    {
        scan->tok = scan->look;
        _get_char(scan);
        skip_white(scan);
        return;
    }
    if (is_digit(scan->look)) 
    {
        scan->tok = TOK_NUM;
        scan->value.num = extract_num(scan);
        return;
    }
    if (scan->size > 1)
    {
        next = tstr_peek(&scan->code, scan->pc);
        if (scan->size > 2)
        {
	    next2 = tstr_peek(&scan->code, scan->pc + 1);
            if (scan->size > 3)
		next3 = tstr_peek(&scan->code, scan->pc + 2);
        }
    }
    switch (scan->look)
    {
    case '+':
    case '-':
    case '&':
    case '|':
    case '>':
    case '<':
    case '=':
        if (next == scan->look)
            scan->tok |= DOUBLE;
    case '*':
    case '/':
    case '%':
    case '~':
    case '!':
    case '^':
        scan->tok |= scan->look;
        if (next == '=' && scan->look != '=')
            scan->tok |= EQ;
        if ((scan->tok == TOK_IS_EQ || scan->tok == TOK_NOT_EQ) && 
            next2 == '=')
        {
            scan->tok |= STRICT;
        }
        if (scan->tok == TOK_SHR && next2 == '>')
            scan->tok |= TRIPPLE;

        if ((scan->tok == TOK_SHR || scan->tok == TOK_SHL) && next2 == '=')
            scan->tok |= EQ;
        if (scan->tok == TOK_SHRZ && next3 == '=')
            scan->tok |= EQ;

        _get_char(scan);
        if (scan->tok & (DOUBLE | EQ))
            _get_char(scan);
        if (scan->tok & (STRICT | TRIPPLE) ||
            ((scan->tok & (DOUBLE | EQ)) == (DOUBLE | EQ)))
        {
            _get_char(scan);
        }

        if ((scan->tok & (TRIPPLE | DOUBLE | EQ)) == (TRIPPLE | DOUBLE | EQ))
            _get_char(scan);

        skip_white(scan);
        return;
    }
    if (is_string_delim(scan->look))
    {
        scan->tok = TOK_STRING;
        scan->value.string = extract_string(scan);
        return;
    }
    if (is_valid_identifier_first_letter(scan->look))
    {
        int constant;
        tstr_t id = extract_identifier(scan);

        if (g_get_constants_cb && !g_get_constants_cb(&constant, &id))
        {
            scan->tok = TOK_CONSTANT;
            scan->value.constant = constant;
        }
        else
        {
            scan->tok = identifier_to_tok(&id);
            if (scan->tok == TOK_ID)
                scan->value.identifier = id;
        }
        return;
    }
    if (IS_EOF(scan))
    {
        scan->tok = TOK_EOF;
        return;
    }
    /* Unknown character, just skip it */
    _get_char(scan);
}

static char *tok_to_str(token_type_t tok)
{
    static char s[3] = { '\'', '\0', '\'' };

    switch (tok)
    {
    case TOK_NUM:
        return "number";
    case TOK_ID:
        return "identifier";
    default:
        break;
    }
    s[1] = tok & 0xff;
    return s;
}

void js_scan_trace(scan_t *scan)
{
    int p;

    for (p = scan->trace_point; p - scan->lpc < scan->size; p++)
    {
        char c = tstr_peek(&scan->code, p);

        if (c == '\n' || c == '\r')
            break;

        tp_out("%c", c);
    }
    tp_out("\n");
    for (p = scan->trace_point; p < scan->last_token_start; p++)
        tp_out(" ", tstr_peek(&scan->code, p));
    tp_out("^\n");
}

static int scan_failure(scan_t *scan, token_type_t expected)
{
    tp_out("ERROR: expected %s\n", tok_to_str(expected));
    js_scan_trace(scan);
    return -1;
}

int _js_scan_match(scan_t *scan, token_type_t tok)
{
    /* Poor man's semicolon insertion - only on EOF */
    if (tok == TOK_END_STATEMENT && scan->tok == TOK_EOF)
        return 0;

    if (scan->tok != tok)
        return scan_failure(scan, tok);

    js_scan_next_token(scan);
    return 0;
}

int js_scan_get_identifier(scan_t *scan, tstr_t *id)
{
    if (scan->tok != TOK_ID)
        return scan_failure(scan, TOK_ID);
    
    *id = tstr_dup(scan->value.identifier);

    js_scan_next_token(scan);
    return 0;
}

int js_scan_get_string(scan_t *scan, tstr_t *str)
{
    tstr_t *src;

    if (scan->tok != TOK_STRING)
        return scan_failure(scan, TOK_STRING);

    src = &scan->value.string;

    if (TSTR_IS_ESCAPED(src))
        tstr_unescape(str, src);
    else
        *str = tstr_dup(*src);

    js_scan_next_token(scan);
    return 0;
}

int js_scan_get_num(scan_t *scan, tnum_t *ret)
{
    int rc = -1;

    if (scan->tok != TOK_NUM || scan->flags & SCAN_FLAG_INVALID)
    {
        scan_failure(scan, TOK_NUM);
        goto Exit;
    }

    *ret = scan->value.num;
    rc = 0;

Exit:
    js_scan_next_token(scan);
    return rc;
}

int js_scan_get_constant(scan_t *scan)
{
    int ret = scan->value.constant;
    js_scan_next_token(scan);
    return ret;
}

int js_scan_get_remaining(scan_t *scan)
{
    return scan->size;
}

token_group_t js_scan_get_token_group(scan_t *scan)
{
    switch (scan->tok)
    {
    case TOK_VAR: 
    case TOK_OPEN_SCOPE:
    case TOK_CLOSE_SCOPE:
    case TOK_OPEN_MEMBER:
    case TOK_CLOSE_MEMBER:
    case TOK_FUNCTION:
    case TOK_THIS:
        return TOKEN_GRP_SCOPE;
    case TOK_FOR:
    case TOK_WHILE:
    case TOK_ELSE:
    case TOK_RETURN:
    case TOK_IF:
    case TOK_CONTINUE:
    case TOK_BREAK:
    case TOK_NEW:
    case TOK_NULL:
    case TOK_UNDEFINED:
    case TOK_THROW:
    case TOK_IN:
    case TOK_TRY:
    case TOK_CATCH:
        return TOKEN_GRP_CONTROL;
    case TOK_NUM:
    case TOK_STRING:
    case TOK_TRUE:
    case TOK_FALSE:
    case TOK_ARGUMENTS:
        return TOKEN_GRP_DATA;
    case TOK_CONSTANT:
        return TOKEN_GRP_CONSTANT;
    default:
        break;
    }
    return TOKEN_GRP_NONE;
}

scan_t *js_scan_save(scan_t *scan)
{
    scan_t *copy = tmalloc_type(scan_t);

    *copy = *scan;
    copy->internal_buf = NULL; /* Only one is in-charge of a sliced buf */
    return copy;
}

void js_scan_restore(scan_t *dst, scan_t *src)
{
    tstr_t *internal_buf = dst->internal_buf;

    *dst = *src;
    dst->internal_buf = internal_buf;
}

scan_t *js_scan_slice(scan_t *start, scan_t *end)
{
    scan_t *ret = js_scan_save(start);

    ret->size = end->lpc - start->lpc;
    if (TSTR_IS_ALLOCATED(&start->code))
    {
        ret->code = tstr_slice(&start->code, start->lpc, ret->size);
        ret->internal_buf = &ret->code;
        ret->lpc = ret->pc = ret->last_token_start = ret->trace_point = 0;
        ret->pc += start->pc - start->lpc;
    }
    return ret;
}

void js_scan_free(scan_t *scan)
{
    if (!scan)
        return;

    if (scan->internal_buf)
        tstr_free(scan->internal_buf);
    tfree(scan);
}

void js_scan_set_trace_point(scan_t *scan)
{
    scan->trace_point = scan->last_token_start;
}

void js_scan_uninit(scan_t *scan)
{
    js_scan_free(scan);
}

scan_t *_js_scan_init(tstr_t *data, int own_data)
{
    scan_t *scan = tmalloc_type(scan_t);

    scan->code = *data;
    scan->last_token_start = scan->trace_point = scan->pc = 0;
    scan->size = data->len + 1;
    scan->look = 255;
    scan->flags = 0;
    scan->internal_buf = own_data ? &scan->code : 0;
    _get_char(scan);
    skip_white(scan);
    js_scan_next_token(scan);
    return scan;
}

void js_scan_set_constants_cb(get_constants_cb_t cb)
{
    g_get_constants_cb = cb;
}
