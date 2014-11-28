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
#include "util/cli.h"
#include "js/js_eval.h"
#include "js/js.h"

#ifdef CONFIG_CLI_SYNTAX_HIGHLIGHTING

#define COLOR(code) CTRL(code)

/* VT 100 control codes.
 * XXX: should be consts
 * XXX: should move to a different file
 */
/* Not using real "dim" since screen doesn't seem to like it. paint it gray */
static char TERM_COLOR_DIM[] = { 0x1b, '[', '9', '0', 'm' };
static char TERM_COLOR_RED[] = { 0x1b, '[', '3', '1', 'm' };
static char TERM_COLOR_CYAN[] = { 0x1b, '[', '3', '6', 'm' };
static char TERM_COLOR_MAGENTA[] = { 0x1b, '[', '3', '5', 'm' };
static char TERM_COLOR_BLUE[] = { 0x1b, '[', '3', '4', 'm' };
static char TERM_COLOR_RESET[] = { 0x1b, '[', '0', 'm' };

static void repl_syntax_highlight(tstr_t *line)
{
    scan_t *s;
    int last_offset = 0;

    s = js_scan_init(line);
    while ((CUR_TOK(s)) != TOK_EOF)
    {
        int offset;

        switch (js_scan_get_token_group(s))
        {
        case TOKEN_GRP_SCOPE:
            COLOR(TERM_COLOR_CYAN); 
            break;
        case TOKEN_GRP_CONTROL:
            COLOR(TERM_COLOR_RED);
            break;
        case TOKEN_GRP_DATA:
            COLOR(TERM_COLOR_MAGENTA);
            break;
        case TOKEN_GRP_CONSTANT:
            COLOR(TERM_COLOR_BLUE);
            break;
        default:
            break;
        }

        offset = line->len - js_scan_get_remaining(s);
        console_write(TPTR(line) + last_offset, offset - last_offset);
        COLOR(TERM_COLOR_RESET);
        last_offset = offset;
        js_scan_next_token(s);
    }
    js_scan_uninit(s);
}
#else
#define COLOR(code)
#endif

static void repl_process_line(tstr_t *line)
{
    obj_t *o = UNDEF;
    int rc = 0, rank;
    static tstr_t multiline;
    tstr_t *to_eval;

    if (multiline.len)
    {
        tstr_t old = multiline;

        tstr_cat(&multiline, &old, line);
        tstr_free(&old);
        to_eval = &multiline;
    }
    else
        to_eval = line;

    rank = js_eval_rank(*to_eval);
    if (rank > 0)
    {
        if (!multiline.len)
            multiline = tstr_dup(*line);
        cli_prompt_set(".", rank + 2);
        return;
    }

    rc = js_eval(&o, to_eval);
    if (rc)
    {
        COLOR(TERM_COLOR_RED);
        console_printf("%o\n", o);
    }
    else
    {
        COLOR(TERM_COLOR_DIM);
        console_printf("= %o\n", o);
    }

    COLOR(TERM_COLOR_RESET);
    obj_put(o);

    if (multiline.len)
    {
        tstr_free(&multiline);
	multiline.len = 0;
    }
    cli_prompt_set(NULL, 0);
}

static cli_client_t repl_cli_client = {
    .process_line = repl_process_line,
    .signal = js_eval_stop_execution,
#ifdef CONFIG_CLI_SYNTAX_HIGHLIGHTING
    .syntax_hightlight = repl_syntax_highlight,
#endif
};

void app_start(int argc, char *argv[])
{
    console_printf("Application - JS Console\n");

    cli_start(&repl_cli_client);
}
