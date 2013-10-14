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
#include "util/tp_types.h"
#include "util/debug.h"
#include "util/tprintf.h"
#include "util/tnum.h"
#include <stdarg.h>

static struct {
    char c;
    tprintf_handler_t h;
} tprintf_handlers[3];

static int num_tprintf_handlers = 0;

#define PRINT_SINGLE(c) do { \
    char b[1]; \
    b[0] = c; \
    printer->print(printer, b, 1); \
} while (0)

void tprint_integer(printer_t *printer, int num, int base, int min_digits)
{
    char nbuf[40], *p = nbuf;
    char const hex2ascii[] = "0123456789abcdef";

    /* Copy number to nbuf in reverse, null terminated order */
    *p = '\0';
    do 
    {
	*++p = hex2ascii[num % base];
	min_digits--;
    } while (num /= base);

    while (min_digits-- > 0)
	*++p = '0';

    /* Output our number */
    while (*p)
	PRINT_SINGLE(*p--);
}

static void print_fp(printer_t *printer, double num, unsigned dec_digits)
{
    unsigned int mult = 1, integer, frac;

    if (num < 0.0) 
    {
	num = -num;
	PRINT_SINGLE('-');
    }

    mult = exp_power(dec_digits);

    /* round by adding .5 LSB */
    if (dec_digits < 8)
	num += 0.5 / mult;

    integer = (unsigned int)num;
    frac = (unsigned int)((num - integer) * mult);

    /* print integer portion */
    tprint_integer(printer, integer, 10, 0);

    PRINT_SINGLE('.');

    /* print fractional portion */
    tprint_integer(printer, frac, 10, dec_digits);
}

static void print_integer(printer_t *printer, int num, int base, int sign)
{
    if (sign && num < 0) 
    {
	num = -num;
	PRINT_SINGLE('-');
    }

    tprint_integer(printer, num, base, 0);
}

void vtprintf(printer_t *printer, char *fmt, va_list ap)
{
    char *p, *percent;
    int is_long, c, i, base, ignore_modifiers = 0;
    unsigned long num;

    while(1)
    {
	while ((c = *fmt++) != '%' || ignore_modifiers)
	{
	    if (c == '\0')
		return;
	    PRINT_SINGLE(c);
	}

	percent = fmt - 1;
	is_long = 0;

again:	
	switch (c = *fmt++) 
	{
	case 'l': is_long = 1; goto again;
	case '%': PRINT_SINGLE(c); break;
	case 'c': PRINT_SINGLE(va_arg(ap, int)); break;
	case 'f': print_fp(printer, va_arg(ap, double), 6); break;
	case 's':
	    p = va_arg(ap, char *);
	    if (p == NULL)
		p = "(null)";
	    printer->print(printer, p, strlen(p));
	    break;
	case 'd':
	    base = 10;
	    num = is_long ? va_arg(ap, long) : va_arg(ap, int);
	    print_integer(printer, num, base, 1);
	    break;
	case 'p':
	    base = 16;
	    num = (uint_ptr_t)va_arg(ap, void *);
	    print_integer(printer, num, base, 0);
	    break;
	case 'u':
	    base = 10;
	    num = is_long ? va_arg(ap, unsigned long) : 
		va_arg(ap, unsigned int);
	    print_integer(printer, num, base, 0);
	    break;
	case 'x':
	    base = 16;
	    num = is_long ? va_arg(ap, unsigned long) : 
		va_arg(ap, unsigned int);
	    print_integer(printer, num, base, 0);
	    break;
	default:
	    for (i = 0; i < num_tprintf_handlers && 
		    fmt[-1] != tprintf_handlers[i].c; i++);

	    if (i == num_tprintf_handlers)
	    {
		/* No format handler was found. Ignore modifiers for
		 * the reset of the fmt string.
		 */
		printer->print(printer, percent, fmt - percent);
		ignore_modifiers = 1;
		break;
	    }
	    tprintf_handlers[i].h(printer, va_arg(ap, void *));
	    break;
	}
    }
}

typedef struct {
    printer_t printer;
    char *buf;
    int count;
    int max;
} tsnprintf_ctx_t;

static int tsnprintf_print(printer_t *printer, char *buf, int size)
{
    tsnprintf_ctx_t *ctx = (tsnprintf_ctx_t *)printer;

    while (size--)
    {
	if (ctx->max > 0)
	{
	    ctx->buf[ctx->count] = *buf++;
	    ctx->max--;
	}
	ctx->count++;
    }
    return 0;
}

int vtsnprintf(char *buf, int n, char *fmt, va_list ap)
{
    tsnprintf_ctx_t ctx;

    ctx.printer.print = tsnprintf_print;
    ctx.buf = buf;
    ctx.count = 0;
    ctx.max = n;

    vtprintf(&ctx.printer, fmt, ap);

    /* NULL terminate */
    if (ctx.max)
	ctx.buf[ctx.count] = '\0';

    return ctx.count;
}

int tsnprintf(char *buf, int n, char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vtsnprintf(buf, n, fmt, ap);
    va_end(ap);
    return ret;
}

void tprintf(printer_t *printer, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vtprintf(printer, fmt, ap);
    va_end(ap);
}

void tprintf_register_handler(char c, tprintf_handler_t h)
{
    if (num_tprintf_handlers == sizeof(tprintf_handlers) / 
	sizeof(tprintf_handlers[0]))
    {
	tp_crit(("Number of tprintf handlers exceeded\n"));
    }

    tprintf_handlers[num_tprintf_handlers].c = c;
    tprintf_handlers[num_tprintf_handlers].h = h;
    num_tprintf_handlers++;
}
