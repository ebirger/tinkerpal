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
#include "util/tstr.h"
#include "util/event.h"
#include "util/debug.h"
#include "main/console.h"
#ifdef CONFIG_JS
#include "js/js_eval.h"
#include "js/js.h"
#endif
#include "apps/history.h"
#include <string.h> /* memcpy - can we avoid this? */

/* VT 100 control codes.
 * XXX: should be consts
 * XXX: should move to a different file
 */
static char TERM_BS[] = { '\b', ' ', '\b' };
static char TERM_CURSOR_LEFT[] = { '\b' };
static char CRLF[] = { '\r', '\n'};
/* Not using real "dim" since screen doesn't seem to like it. paint it gray */
#ifdef CONFIG_JS
static char TERM_COLOR_DIM[] = { 0x1b, '[', '9', '0', 'm' };
static char TERM_COLOR_RED[] = { 0x1b, '[', '3', '1', 'm' };
#ifdef CONFIG_APP_CLI_SYNTAX_HIGHLIGHTING
static char TERM_SAVE_CURSOR[] = { 0x1b, '[', 's' };
static char TERM_RESTORE_CURSOR[] = { 0x1b, '[', 'u' };
static char TERM_COLOR_CYAN[] = { 0x1b, '[', '3', '6', 'm' };
static char TERM_COLOR_MAGENTA[] = { 0x1b, '[', '3', '5', 'm' };
static char TERM_COLOR_BLUE[] = { 0x1b, '[', '3', '4', 'm' };
#endif
static char TERM_COLOR_RESET[] = { 0x1b, '[', '0', 'm' };
#endif

static char SPACE[] = { ' ' };
static char prompt[] = { 'T','i','n','k','e','r','P','a','l','>',' ' };

static char cli_buf[CONFIG_APP_CLI_HISTORY_BUFFER_SIZE];
static char *buf, *read_buf;
static int free_size = sizeof(cli_buf), size, cur_line_pos;
static tstr_t cur_line = {}, history = {};

#define BUF_START (cli_buf + sizeof(line_desc_t))

#define CTRL(c) console_write(c, sizeof(c))

static void reset_line(void)
{
    int i;

    /* Clear to end of line */
    for (i = cur_line_pos; i < cur_line.len; i++)
	CTRL(SPACE);
    /* Clear to start of line */
    for (i = 0; i < cur_line.len; i++)
	CTRL(TERM_BS);
}

static void syntax_hightlight(void)
{
#ifdef CONFIG_APP_CLI_SYNTAX_HIGHLIGHTING
    scan_t *s;
    int last_offset = 0, pos = cur_line_pos;

    CTRL(TERM_SAVE_CURSOR);

    /* Move to start of line */
    while (pos--)
	CTRL(TERM_CURSOR_LEFT);

    s = js_scan_init(&cur_line);
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

	offset = cur_line.len - js_scan_get_remaining(s);
	console_write(cur_line.value + last_offset, offset - last_offset);
	CTRL(TERM_COLOR_RESET);
	last_offset = offset;
	js_scan_next_token(s);
    }
    js_scan_uninit(s);
    CTRL(TERM_RESTORE_CURSOR);
#endif
}

static void read_ack(void)
{
    free_size -= size;
    cur_line.len += size;
    cur_line_pos += size;
    buf += size;
    read_buf = buf;
    size = 0;
}

static void roll_back(void)
{
    free_size += cur_line.len;
    read_buf = buf = cur_line.value;
    reset_line();
    cur_line_pos = cur_line.len = 0;
}

static void output_history(void)
{
    roll_back();
    
    size = history_get(&history, buf, free_size);
    
    console_write(buf, size);
    read_ack();
}

static void do_up(void)
{
    if (history_is_first(&history))
	return;

    history_prev(&history);
    output_history();
}

static void do_down(void)
{
    if (history_is_last(&history))
	return;

    history_next(&history);
    output_history();
}

static void do_left(void)
{
    if (!cur_line_pos)
	return;

    cur_line_pos--;
    CTRL(TERM_CURSOR_LEFT);
}

static void do_right(void)
{
    char b;

    if (cur_line_pos == cur_line.len)
	return;

    /* Write current character */
    b = *(cur_line.value + cur_line_pos);
    console_write(&b, 1);
    cur_line_pos++;
}

static void do_bs(void)
{
    int delta;

    if (!cur_line.len || !cur_line_pos)
	return;

    /* Move back */
    CTRL(TERM_CURSOR_LEFT);

    delta = cur_line.len - cur_line_pos;
    if (delta)
    {
	char *cur = cur_line.value + cur_line_pos;

	memmove(cur - 1, cur, delta);
	console_write(cur - 1, delta);
    }

    /* Erase last character */
    CTRL(SPACE);
    CTRL(TERM_CURSOR_LEFT);

    /* Reset cursor to its original position */
    while (delta--)
	CTRL(TERM_CURSOR_LEFT);

    size = -1;
    read_ack();
}

static void do_home(void)
{
    while (cur_line_pos)
	do_left();
}

static void do_end(void)
{
    while (cur_line_pos < cur_line.len)
	do_right();
}

static void do_del(void)
{
    if (cur_line_pos == cur_line.len)
	return;

    do_right();
    do_bs();
}

static void app_quit(void)
{
    event_timer_del_all();
    event_watch_del_all();
}

static void do_esc(void)
{
    if ((read_buf - buf) + size < 3)
    {
	/* Wait for more */
	read_buf += size;
	return;
    }

    size += (read_buf - buf);
    read_buf = buf;
    switch (buf[1])
    {
    case '[':
	switch (buf[2])
	{
	case 'A': do_up(); return;
	case 'B': do_down(); return;
	case 'C': do_right(); return;
	case 'D': do_left(); return;
	case '1': if (size == 4 && buf[3] == '~') do_home(); return;
	case '3': if (size == 4 && buf[3] == '~') do_del(); return;
	case '4': if (size == 4 && buf[3] == '~') do_end(); return;
	}
	break;
    case 'O':
	switch (buf[2])
	{
	case 'H': do_home(); return;
	case 'F': do_end(); return;
	}
	break;
    }
}

static void write_buf(void)
{
    int delta, output_size = size;
    char *output = buf;

    delta = cur_line.len - cur_line_pos;
    if (delta)
    {
	output -= delta;
	output_size += delta;
	memmove(output + size, output, delta + size);
	memmove(output, buf + size, size);
    }
    console_write(output, output_size);

    /* Reset cursor to its original position */
    while (delta--)
	CTRL(TERM_CURSOR_LEFT);
    read_ack();
}

static void on_event(event_watch_t *ew, int id)
{
    tstr_t quit_cmd = S("quit");

    size = console_read(read_buf, free_size);
    /* Assumptions: 
     * We do not handle control characters in mid buf -
     *  we assume we get here quickly enough.
     */
    switch (buf[0])
    {
    case 0x7f:
    case '\b':
	do_bs();
	syntax_hightlight();
	return;
    case 0x1b:
	do_esc();
	syntax_hightlight();
	return;
    case '\r':
    case '\n':
	CTRL(CRLF);
	/* Got line, lets go right ahead and eval it */
	break;
    default:
	write_buf();
	syntax_hightlight();
	/* Continue accumulating input */
	return;
    }

    if (!tstr_cmp(&cur_line, &quit_cmd))
    {
	app_quit();
	return;
    }

    if (cur_line.len)
    {
#ifdef CONFIG_JS
	obj_t *o;
	int rc;

	rc = js_eval(&o, &cur_line);
	if (rc)
	{
	    CTRL(TERM_COLOR_RED);
	    tp_out(("%o\n", o));
	}
	else
	{
	    CTRL(TERM_COLOR_DIM);
	    tp_out(("= %o\n", o));
	}

	CTRL(TERM_COLOR_RESET);
	obj_put(o);
#else
	console_printf("Got %S\n", &cur_line);
#endif
	history_commit(&history, &cur_line);
	cur_line_pos = 0;
	read_buf = buf = cur_line.value;
    }

    console_write(prompt, sizeof(prompt));
}

static event_watch_t cli_event_watch = {
    .watch_event = on_event,
};

void app_start(int argc, char *argv[])
{
    tp_out(("Application - CLI\n"));

    console_write(prompt, sizeof(prompt));
    read_buf = buf = cur_line.value = BUF_START;
    history_init(&history, BUF_START);
    TSTR_SET_ALLOCATED(&cur_line);
    console_event_watch_set(&cli_event_watch);
}
