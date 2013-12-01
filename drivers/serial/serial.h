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

static inline int serial_read(int id, char *buf, int size)
{
    if (RES_BASE(id) != UART_RESOURCE_ID_BASE)
	return -1;

    return platform.serial.read(RES_ID(id), buf, size);
}

static inline int serial_write(int id, char *buf, int size)
{
    if (RES_BASE(id) != UART_RESOURCE_ID_BASE)
	return -1;
    
    return platform.serial.write(RES_ID(id), buf, size);
}

int serial_enable(int id, int enabled);

static inline int serial_default_console_id(void)
{
    return RES(UART_RESOURCE_ID_BASE, platform.serial.default_console_id);
}

/* XXX: should receive tstr */
static inline int serial_get_constant(int *constant, char *buf, int len)
{
#define SERIAL_PREFIX "UART"

    if (len < sizeof(SERIAL_PREFIX) - 1 ||
	prefix_comp(sizeof(SERIAL_PREFIX) - 1, SERIAL_PREFIX, buf))
    {
	return -1;
    }

    buf += sizeof(SERIAL_PREFIX) - 1;
    len -= sizeof(SERIAL_PREFIX) - 1;

    if (len != 1)
	return -1;

    *constant = RES(UART_RESOURCE_ID_BASE, buf[0] - '0');
    return 0;
}

/* Used by the platform to set/test serial events */
static inline void serial_mark_on(int u, void (*event_mark_on)(int id))
{
    event_mark_on(RES(UART_RESOURCE_ID_BASE, u));
}

#ifdef CONFIG_BUFFERED_SERIAL

int buffered_serial_push(int u, char c);
int buffered_serial_events_process(void (*mark_on)(int id));
int buffered_serial_read(int u, char *buf, int size);

#endif

#endif
