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
#include "main/console.h"
#include "boards/board.h"
#include "drivers/serial/serial.h"
#include <stdarg.h>

static resource_t console_id;
static int console_event_id = -1;
static event_t *user_e;

/* Stupid wrapper for user_e so their "free" cb is never called */
static void console_watch_event(event_t *e, u32 resource_id)
{
    user_e->trigger(user_e, resource_id);
}

static event_t console_e = { .trigger = console_watch_event };

int console_read(char *buf, int size)
{
    tp_assert(console_id);
    return serial_read(console_id, buf, size);
}

int console_write(char *buf, int size)
{
    tp_assert(console_id);
    for (; size--; buf++)
    {
        if (*buf == '\n')
	{
	    char b = '\r';
	    serial_write(console_id, &b, 1);
	}

	serial_write(console_id, buf, 1);
    }
    return 0;
}

static int console_printer_write(printer_t *printer, char *buf, int size)
{
    return console_write(buf, size);
}

void console_event_watch_set(event_t *e)
{
    if (console_event_id != -1)
	event_watch_del(console_event_id);
    user_e = e;
    console_event_id = event_watch_set(console_id, &console_e);
}

/* XXX: this doesn't really belong here */
static void str_dump(printer_t *printer, void *o)
{
    tstr_t *s = o;
    int i;

    for (i = 0; i < s->len; i++)
    {
	char c = TPTR(s)[i];

	if (c == '\n')
	    tprintf(printer, "\\n");
	else if (c == '\t')
	    tprintf(printer, "\\t");
	else if (c == '\r')
	    tprintf(printer, "\\r");
	else if (c == '"')
	    tprintf(printer, "\\\"");
	else if (c == '\0')
	    tprintf(printer, "\\0");
	else if (c == '\\')
	    tprintf(printer, "\\");
	else if (c >= 0x20 && c <= 0x7E)
	    tprintf(printer, "%c", c);
	else
	    tprintf(printer, "\\u00%s%x", c <= 0xf ? "0" : "", c);
    }
}

void console_set_id(resource_t id)
{
    console_id = id;
    /* refresh event listener */
    if (user_e)
	console_event_watch_set(user_e);
}

static printer_t console_printer = {
    .print = console_printer_write,
};

void console_printf(char *fmt, ...)
{ 
    va_list ap;

    va_start(ap, fmt);
    vtprintf(&console_printer, fmt, ap);
    va_end(ap);
}

void console_init(void)
{
    resource_t id = board.default_console_id;

    serial_enable(id, 1);
    console_set_id(id);
    tprintf_register_handler('S', str_dump);
}
