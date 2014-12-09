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
#include "util/history.h"
#include "util/cli.h"
#include "main/console.h"
#include <string.h> /* memcpy - can we avoid this? */

/* VT 100 control codes.
 * XXX: should be consts
 * XXX: should move to a different file
 */
static char TERM_BS[] = { '\b', ' ', '\b' };
static char TERM_CURSOR_LEFT[] = { '\b' };
static char CRLF[] = { '\r', '\n'};
static char SPACE[] = { ' ' };
#ifdef CONFIG_CLI_SYNTAX_HIGHLIGHTING
static char TERM_SAVE_CURSOR[] = { 0x1b, '[', 's' };
static char TERM_RESTORE_CURSOR[] = { 0x1b, '[', 'u' };
#endif
static char def_prompt[] = "TinkerPal>";
static char *g_prompt = def_prompt;
static int g_prompt_repetitions = 1;

static char cli_buf[CONFIG_CLI_BUFFER_SIZE];
static char *buf, *read_buf;
static int free_size = sizeof(cli_buf), size, cur_line_pos;
static tstr_t cur_line = {};
static cli_client_t *g_client;
history_t *history;

static void output_prompt(void)
{
    int i, len = strlen(g_prompt);

    for (i = 0; i < g_prompt_repetitions; i++)
        console_write(g_prompt, len);
    CTRL(SPACE);
}

static void reset_line(void)
{
    int i;

    /* Clear to end of line */
    for (i = cur_line.len - cur_line_pos; i; i--)
        CTRL(SPACE);
    /* Clear to start of line */
    for (i = cur_line.len; i; i--)
        CTRL(TERM_BS);
}

static void syntax_hightlight(void)
{
#ifdef CONFIG_CLI_SYNTAX_HIGHLIGHTING
    int pos = cur_line_pos;

    CTRL(TERM_SAVE_CURSOR);

    /* Move to start of line */
    while (pos--)
        CTRL(TERM_CURSOR_LEFT);

    if (g_client->syntax_hightlight)
        g_client->syntax_hightlight(&cur_line);

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
    reset_line();
    read_buf -= cur_line_pos;
    buf -= cur_line_pos;
    cur_line_pos = cur_line.len = 0;

}

static void output_history(void)
{
    roll_back();
    
    size = history_get(history, buf, free_size);
    
    console_write(buf, size);
    read_ack();
}

static void do_up(void)
{
    history_prev(history);
    output_history();
}

static void do_down(void)
{
    history_next(history);
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
    if (cur_line_pos == cur_line.len)
        return;

    /* Write current character */
    tstr_dump(&cur_line, cur_line_pos, 1, console_write);
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
        tstr_move(&cur_line, cur_line_pos - 1, cur_line_pos, delta);
        tstr_dump(&cur_line, cur_line_pos - 1, delta, console_write);
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
    history_free(history);
    if (g_client->quit)
        g_client->quit();
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

static void on_event(event_t *e, u32 id, u64 timestamp)
{
    tstr_t *quit_cmd = &S("quit"), *history_cmd = &S("history");

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

    if (!tstr_cmp(&cur_line, quit_cmd))
    {
        app_quit();
        return;
    }

    if (cur_line.len)
    {
        if (!tstr_cmp(&cur_line, history_cmd))
            history_dump(history);
        else if (g_client->process_line)
            g_client->process_line(&cur_line);
        history_commit(history, &cur_line);
        cur_line_pos = 0;
        read_buf = buf = cli_buf;
        tstr_init(&cur_line, cli_buf, 0, TSTR_FLAG_ALLOCATED);
    }

    output_prompt();
}

static void on_signal(event_t *e, u32 id, u64 timestamp)
{
    if (g_client->signal)
	g_client->signal();
}

static event_t cli_event = {
    .trigger = on_event,
    .signal = on_signal,
};

void cli_prompt_set(char *p, int repetitions)
{
    if (p)
    {
        g_prompt = p;
        g_prompt_repetitions = repetitions;
    }
    else
    {
        g_prompt = def_prompt;
        g_prompt_repetitions = 1;
    }
}

void cli_start(cli_client_t *client)
{
    g_client = client;
    output_prompt();
    read_buf = buf = cli_buf;
    tstr_init(&cur_line, cli_buf, 0, TSTR_FLAG_ALLOCATED);
    history = history_new();
    console_event_watch_set(&cli_event);
}
