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
#include <ctype.h>
#include "util/tmalloc.h"
#include "util/tnum.h"
#include "util/debug.h"
#include "js/js_scan.h"

typedef union {
    tstr_t identifier;
    tstr_t string;
    tnum_t num;
    int constant;
} scan_value_t;

struct scan_t {
    token_type_t tok; /* Must be first */
    char *lpc;
    char *pc;
    char *trace_point;
    char *last_token_start;
    int size; /* should be size_t */
    char look;
#define SCAN_FLAG_EOF 0x0001
#define SCAN_FLAG_ALLOCED 0x0002
#define SCAN_FLAG_INVALID 0x0004
    unsigned short flags;
    scan_value_t value;
};

static get_constants_cb_t g_get_constants_cb;

#define IS_EOF(scan) ((scan)->flags & SCAN_FLAG_EOF)
#define SET_EOF(scan) ((scan)->flags |= SCAN_FLAG_EOF)
#define IS_ALLOCED(scan) ((scan)->flags & SCAN_FLAG_ALLOCED)

static inline int is_number_letter(char c)
{
    return isxdigit((int)c) || c == '.' || c == 'e' || c == 'x';
}

static inline int is_valid_identifier_first_letter(char c)
{
    return isalpha((int)c) || c == '_' || c == '$';
}

static inline int is_valid_identifier_non_first_letter(char c)
{
    return is_valid_identifier_first_letter(c) || isdigit((int)c);
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
	    scan->look = *scan->pc;
	scan->pc++;
	scan->size--;
    }
    if (scan->size == 0)
    {
	scan->look = 255; /* invalid char */
	SET_EOF(scan);
	return;
    }
    tp_debug(("[%x:%c] ", scan->look, scan->look));
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
	next = *scan->pc;
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
	    while (scan->look != '*' && *scan->pc != '/' && !IS_EOF(scan))
		_get_char(scan);
	    _get_char(scan); /* Skip '*' */
	    _get_char(scan); /* Skip '/' */
	}
	else
	    break;
    }
}

static inline tstr_t extract_string(scan_t *scan)
{
    tstr_t ret = {};
    token_type_t delim = scan->look;

    _get_char(scan); /* skip enclosure */
    ret.value = scan->lpc;
    ret.flags = 0;
    while (scan->look != delim && !IS_EOF(scan))
    {
	if (is_newline(scan->look))
	    tp_crit(("Newlines are not allowed in strings\n"));

	if (scan->look == '\\')
	    TSTR_SET_ESCAPED(&ret);

	/* XXX: allow escaping the delimiter */
	_get_char(scan);
    }

    ret.len = scan->lpc - ret.value;
    if (IS_ALLOCED(scan))
	TSTR_SET_ALLOCATED(&ret);
    _get_char(scan); /* skip enclosure */
    skip_white(scan);
    return ret;
}

static inline tstr_t extract_identifier(scan_t *scan)
{
    tstr_t ret = {};

    ret.value = scan->lpc;
    while (is_valid_identifier_non_first_letter(scan->look))
	_get_char(scan);

    ret.len = scan->lpc - ret.value;
    if (IS_ALLOCED(scan))
	TSTR_SET_ALLOCATED(&ret);
    skip_white(scan);
    return ret;
}

static inline token_type_t identifier_to_tok(const tstr_t *str)
{
    struct {
	tstr_t str;
	token_type_t tok;
    } *k, keywords[] = {
#define K(n, o) { .str = S(n), .tok = o }
	/* Note: keywords are arranged by length for quick escape */
	K("if", TOK_IF),
	K("for", TOK_FOR),
	K("var", TOK_VAR),
	K("new", TOK_NEW),
	K("try", TOK_TRY),
	K("else", TOK_ELSE),
	K("this", TOK_THIS),
	K("true", TOK_TRUE),
	K("null", TOK_NULL),
	K("case", TOK_CASE),
	K("while", TOK_WHILE),
	K("break", TOK_BREAK),
	K("false", TOK_FALSE),
	K("throw", TOK_THROW),
	K("catch", TOK_CATCH),
	K("switch", TOK_SWITCH),
	K("return", TOK_RETURN),
	K("default", TOK_DEFAULT),
	K("function", TOK_FUNCTION),
	K("continue", TOK_CONTINUE),
	K("prototype", TOK_PROTOTYPE),
	K("undefined", TOK_UNDEFINED),
	{}
    };

    for (k = keywords; k->tok; k++)
    {
	/* Quick escape - keywords are arranged by length. No need
	 * to bother comparing if we are already shorter than the current
	 * keyword.
	 */
	if (k->str.len > str->len)
	    return TOK_ID;

	/* Exit on exact match */
	if (!tstr_cmp(&k->str, str))
	    return k->tok;
    }

    return TOK_ID;
}

static inline tnum_t extract_num(scan_t *scan)
{
    tstr_t s;
    tnum_t ret;

    s.flags = 0;
    s.value = scan->lpc;
    while (is_number_letter(scan->look))
	_get_char(scan);
    s.len = scan->lpc - s.value;
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
    char next = 0, next2 = 0;

    scan->tok = 0;
    scan->flags &= ~SCAN_FLAG_INVALID;
    scan->last_token_start = scan->lpc;
    if (scan->size > 1)
    {
	next = *scan->pc;
	if (scan->size > 2)
	    next2 = *(scan->pc + 1);
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

	_get_char(scan);
	if (scan->tok & (DOUBLE | EQ))
	    _get_char(scan);
	if (scan->tok & (STRICT | TRIPPLE))
	    _get_char(scan);
	skip_white(scan);
	return;
    }
    if (is_string_delim(scan->look))
    {
	scan->tok = TOK_STRING;
	scan->value.string = extract_string(scan);
    }
    else if (is_valid_identifier_first_letter(scan->look))
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
    }
    else if (isdigit((int)scan->look)) 
    {
	scan->tok = TOK_NUM;
	scan->value.num = extract_num(scan);
    }
    else if (is_control_char(scan->look))
    {
	scan->tok = scan->look;
	_get_char(scan);
	skip_white(scan);
    }
    else if (IS_EOF(scan))
	scan->tok = TOK_EOF;
    else
    {
	/* Unknown character, just skip it */
	_get_char(scan);
    }
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
    char *p;

    for (p = scan->trace_point; p - scan->pc < scan->size && *p != '\n' && 
	*p != '\r' ; p++)
    {
	tp_out(("%c", *p));
    }
    tp_out(("\n"));
    for (p = scan->trace_point; p < scan->last_token_start; p++)
	tp_out((" ", *p));
    tp_out(("^\n"));
}

static int scan_failure(scan_t *scan, token_type_t expected)
{
    tp_out(("ERROR: expected %s\n", tok_to_str(expected)));
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

int js_scan_get_identifier(tstr_t *id, scan_t *scan)
{
    if (scan->tok != TOK_ID)
	return scan_failure(scan, TOK_ID);
    
    *id = tstr_dup(scan->value.identifier);

    js_scan_next_token(scan);
    return 0;
}

tstr_t js_scan_get_string(scan_t *scan)
{
    tstr_t ret, *src;

    if (scan->tok != TOK_STRING)
	tp_crit(("expected string, got %x:%c\n", scan->tok, scan->tok));

    src = &scan->value.string;

    if (TSTR_IS_ESCAPED(src))
	tstr_unescape(&ret, src);
    else
	ret = tstr_dup(*src);

    js_scan_next_token(scan);
    return ret;
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
    case TOK_TRY:
    case TOK_CATCH:
	return TOKEN_GRP_CONTROL;
    case TOK_NUM:
    case TOK_STRING:
    case TOK_TRUE:
    case TOK_FALSE:
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
    return copy;
}

void js_scan_restore(scan_t *dst, scan_t *src)
{
    *dst = *src;
}

void js_scan_free(scan_t *scan)
{
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

scan_t *js_scan_init(tstr_t *data)
{
    scan_t *scan = tmalloc_type(scan_t);

    scan->last_token_start = scan->trace_point = scan->pc = data->value;
    scan->size = data->len + 1;
    scan->look = 255;
    scan->flags = TSTR_IS_ALLOCATED(data) ? SCAN_FLAG_ALLOCED : 0;
    _get_char(scan);
    skip_white(scan);
    js_scan_next_token(scan);
    return scan;
}

void js_scan_set_constants_cb(get_constants_cb_t cb)
{
    g_get_constants_cb = cb;
}
