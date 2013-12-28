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
#include "drivers/serial/serial.h"
#include "drivers/serial/serial_platform.h"
#include "util/event.h"
#include "mem/tmalloc.h"
#include <string.h> /* memcpy */

#ifdef CONFIG_BUFFERED_SERIAL

typedef struct {
    char buf[64];
    int len;
} uart_buf_t;

static uart_buf_t *uart_bufs[NUM_UARTS] = {};

int buffered_serial_push(int u, char c)
{
    if (uart_bufs[u]->len == sizeof(uart_bufs[u]->buf))
    {
	/* XXX: should we disable uart irq here? */
	return -1;
    }

    uart_bufs[u]->buf[uart_bufs[u]->len] = c;
    uart_bufs[u]->len++;
    return 0;
}

int buffered_serial_events_process(void)
{
    int u, event = 0;

    for (u = 0; u < NUM_UARTS; u++)
    {
	platform.serial.irq_enable(u, 0);
	if ((uart_bufs[u]->len != 0))
	{
	    serial_event_trigger(u);
	    event = 1;
	}
	platform.serial.irq_enable(u, 1);
    }
    return event;
}

int buffered_serial_read(int u, char *buf, int size)
{
    int ret;

    platform.serial.irq_enable(u, 0);
    ret = uart_bufs[u]->len;
    /* XXX: no more than "size */
    memcpy(buf, uart_bufs[u]->buf, uart_bufs[u]->len);
    uart_bufs[u]->len = 0;
    platform.serial.irq_enable(u, 1);
    return ret;
}

int buffered_serial_enable(int u, int enabled)
{
    if (!!uart_bufs[u] == !!enabled)
	return -1;

    if (enabled)
    {
	uart_bufs[u] = tmalloc_type(uart_buf_t);
	uart_bufs[u]->len = 0;
    }
    else
    {
	tfree(uart_bufs[u]);
	uart_bufs[u] = NULL;
    }
    return 0;
}

#endif

void serial_event_trigger(int u)
{
    event_watch_trigger(RES(UART_RESOURCE_ID_BASE, u, 0));
}

int serial_enable(resource_t id, int enabled)
{
    if (RES_BASE(id) != UART_RESOURCE_ID_BASE)
	return -1;

#ifdef CONFIG_BUFFERED_SERIAL
    if (buffered_serial_enable(RES_MAJ(id), enabled))
	return -1;
#endif

    return platform.serial.enable(RES_MAJ(id), enabled);
}
