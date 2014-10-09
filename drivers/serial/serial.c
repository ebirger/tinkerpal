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
#include "util/tprintf.h"
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
    if (c == 0x3 /* CTRL-C */)
    {
	serial_event_signal(u);
	return 0;
    }
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
    event_watch_trigger(UART_RES(u));
}

void serial_event_signal(int u)
{
    event_watch_signal(UART_RES(u));
}

static const serial_driver_t *get_serial_driver(resource_t id)
{
    if (RES_BASE(id) != SERIAL_RESOURCE_ID_BASE)
        return NULL;

    if (RES_MAJ(id) == SERIAL_UART_MAJ)
        return &platform.serial;

#ifdef CONFIG_USB_CDC_ACM
    if (RES_MAJ(id) == SERIAL_USB_MAJ)
    {
        extern serial_driver_t cdc_acm_serial_driver;
        return &cdc_acm_serial_driver;
    }
#endif

    return NULL;
}

int serial_read(resource_t id, char *buf, int size)
{
    const serial_driver_t *driver;

    if (!(driver = get_serial_driver(id)))
        return -1;

    return driver->read(RES_MIN(id), buf, size);
}

int serial_write(resource_t id, char *buf, int size)
{
    const serial_driver_t *driver;

    if (!(driver = get_serial_driver(id)))
        return -1;

    return driver->write(RES_MIN(id), buf, size);
}

typedef struct {
    printer_t printer;
    resource_t id;
} serial_printf_printer_t;

static int serial_printf_print(struct printer_t *printer, char *buf, int size)
{
    serial_printf_printer_t *spp = container_of(printer,
        serial_printf_printer_t, printer);

    serial_write(spp->id, buf, size);
    return 0;
}

void serial_printf(resource_t id, char *fmt, ...)
{
    va_list ap;
    serial_printf_printer_t spp;

    spp.printer.print = serial_printf_print;
    spp.id = id;

    va_start(ap, fmt);
    vtprintf(&spp.printer, fmt, ap);
    va_end(ap);
}

int serial_enable(resource_t id, int enabled)
{
    const serial_driver_t *driver;

    if (!(driver = get_serial_driver(id)))
        return -1;

#ifdef CONFIG_BUFFERED_SERIAL
    /* XXX: support other drivers for buffered serial */
    if (RES_MAJ(id) == SERIAL_UART_MAJ &&
        buffered_serial_enable(RES_MIN(id), enabled))
    {
        return -1;
    }
#endif

    return driver->enable(RES_MIN(id), enabled);
}

int serial_set_params(resource_t id, const serial_params_t *params)
{
    const serial_driver_t *driver;

    if (!(driver = get_serial_driver(id)))
        return -1;

    if (!driver->set_params)
        return -1;

    return driver->set_params(RES_MIN(id), params);
}

static int _serial_get_constant(char *prefix, int maj, int *constant,
    char *buf, int len)
{
    int prefix_len = strlen(prefix);

    if (len < prefix_len || prefix_comp(prefix_len, prefix, buf))
        return -1;

    buf += prefix_len;
    len -= prefix_len;

    if (len != 1)
        return -1;

    *constant = (int)RES(SERIAL_RESOURCE_ID_BASE, maj, buf[0] - '0');
    return 0;
}

int serial_get_constant(int *constant, char *buf, int len)
{
    if (!_serial_get_constant("UART", SERIAL_UART_MAJ, constant, buf, len))
        return 0;
    if (!_serial_get_constant("USB", SERIAL_USB_MAJ, constant, buf, len))
        return 0;
    return -1;
}
