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
#include "apps/cli.h"
#include "js/js_eval.h"
#include "js/js.h"
#include "main/console.h"

/* VT 100 control codes.
 * XXX: should be consts
 * XXX: should move to a different file
 */
/* Not using real "dim" since screen doesn't seem to like it. paint it gray */
static char TERM_COLOR_DIM[] = { 0x1b, '[', '9', '0', 'm' };
static char TERM_COLOR_RED[] = { 0x1b, '[', '3', '1', 'm' };
#ifdef CONFIG_APP_JSCONSOLE_SYNTAX_HIGHLIGHTING
static char TERM_COLOR_CYAN[] = { 0x1b, '[', '3', '6', 'm' };
static char TERM_COLOR_MAGENTA[] = { 0x1b, '[', '3', '5', 'm' };
static char TERM_COLOR_BLUE[] = { 0x1b, '[', '3', '4', 'm' };
#endif
static char TERM_COLOR_RESET[] = { 0x1b, '[', '0', 'm' };

void cli_client_syntax_hightlight(tstr_t *line)
{
#ifdef CONFIG_APP_JSCONSOLE_SYNTAX_HIGHLIGHTING
    scan_t *s;
    int last_offset = 0;

    s = js_scan_init(line);
    while ((CUR_TOK(s)) != TOK_EOF)
    {
	int offset;

	switch (js_scan_get_token_group(s))
	{
	case TOKEN_GRP_SCOPE:
	    CTRL(TERM_COLOR_CYAN); 
	    break;
	case TOKEN_GRP_CONTROL:
	    CTRL(TERM_COLOR_RED);
	    break;
	case TOKEN_GRP_DATA:
	    CTRL(TERM_COLOR_MAGENTA);
	    break;
	case TOKEN_GRP_CONSTANT:
	    CTRL(TERM_COLOR_BLUE);
	    break;
	default:
	    break;
	}

	offset = line->len - js_scan_get_remaining(s);
	console_write(line->value + last_offset, offset - last_offset);
	CTRL(TERM_COLOR_RESET);
	last_offset = offset;
	js_scan_next_token(s);
    }
    js_scan_uninit(s);
#endif
}

void cli_client_process_line(tstr_t *line)
{
    obj_t *o;
    int rc;

    rc = js_eval(&o, line);
    if (rc)
    {
	CTRL(TERM_COLOR_RED);
	console_printf("%o\n", o);
    }
    else
    {
	CTRL(TERM_COLOR_DIM);
	console_printf("= %o\n", o);
    }

    CTRL(TERM_COLOR_RESET);
    obj_put(o);
}

void app_start(int argc, char *argv[])
{
    console_printf("Application - JS Console\n");

    cli_start();
}
