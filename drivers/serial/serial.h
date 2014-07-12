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
#ifndef __DRIVERS_SERIAL_H__
#define __DRIVERS_SERIAL_H__

#include "util/tstr.h"
#include "drivers/resources.h"
#include "platform/platform.h"

#define SERIAL_UART_MAJ 0
#define SERIAL_USB_MAJ 1

#define UART_RES(u) RES(SERIAL_RESOURCE_ID_BASE, SERIAL_UART_MAJ, u)
#define USB_RES RES(SERIAL_RESOURCE_ID_BASE, SERIAL_USB_MAJ, 0)

static inline const serial_driver_t *get_serial_driver(resource_t id)
{
    if (RES_BASE(id) != SERIAL_RESOURCE_ID_BASE)
        return NULL;

    if (RES_MAJ(id) == SERIAL_UART_MAJ)
        return &platform.serial;

    return NULL;
}

static inline int serial_read(resource_t id, char *buf, int size)
{
    const serial_driver_t *driver;

    if (!(driver = get_serial_driver(id)))
        return -1;

    return driver->read(RES_MIN(id), buf, size);
}

static inline int serial_write(resource_t id, char *buf, int size)
{
    const serial_driver_t *driver;

    if (!(driver = get_serial_driver(id)))
        return -1;

    return driver->write(RES_MIN(id), buf, size);
}

int serial_enable(resource_t id, int enabled);

/* XXX: should receive tstr */
static inline int _serial_get_constant(char *prefix, int maj, int *constant,
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

static inline int serial_get_constant(int *constant, char *buf,
    int len)
{
    if (!_serial_get_constant("UART", SERIAL_UART_MAJ, constant, buf, len))
        return 0;
    if (!_serial_get_constant("USB", SERIAL_USB_MAJ, constant, buf, len))
        return 0;
    return -1;
}

#endif
