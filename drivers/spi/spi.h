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
#ifndef __DRIVERS_SPI_H__
#define __DRIVERS_SPI_H__

#include "drivers/resources.h"
#include "platform/platform.h"
#include "util/tstr.h"

#define SPI_RES(port) RES(SPI_RESOURCE_ID_BASE, port, 0)

/* XXX: API should provide error indication
 * CS should be part of the SPI driver */

static inline int spi_init(resource_t port)
{
    if (RES_BASE(port) != SPI_RESOURCE_ID_BASE)
        return -1;

    return platform.spi.init(RES_MAJ(port));
}

static inline void spi_reconf(resource_t port)
{
    if (RES_BASE(port) != SPI_RESOURCE_ID_BASE)
        return;

    platform.spi.reconf(RES_MAJ(port));
}

static inline void spi_set_max_speed(resource_t port, unsigned long speed)
{
    if (RES_BASE(port) != SPI_RESOURCE_ID_BASE)
        return;

    platform.spi.set_max_speed(RES_MAJ(port), speed);
}

static inline void spi_send(resource_t port, unsigned long data)
{
    if (RES_BASE(port) != SPI_RESOURCE_ID_BASE)
        return;

    platform.spi.send(RES_MAJ(port), data);
}

static inline unsigned long spi_receive(resource_t port)
{
    if (RES_BASE(port) != SPI_RESOURCE_ID_BASE)
        return 0;

    return platform.spi.receive(RES_MAJ(port));
}

static inline void spi_receive_mult(resource_t port, u8 buf[], int len)
{
    while (len--)
        *buf++ = (u8)spi_receive(port);
}

static inline void spi_send_mult(resource_t port, u8 buf[], int len)
{
    while (len--)
        spi_send(port, *buf++);
}

static inline int spi_get_constant(int *constant, char *buf, int len)
{
#define SPI_PREFIX "SPI"

    if (len < sizeof(SPI_PREFIX) -1 || 
        prefix_comp(sizeof(SPI_PREFIX) - 1, SPI_PREFIX, buf))
    {
        return -1;
    }

    buf += sizeof(SPI_PREFIX) - 1;
    len -= sizeof(SPI_PREFIX) - 1;

    if (len != 1)
        return -1;

    *constant = (int)RES(SPI_RESOURCE_ID_BASE, buf[0] - '0', 0);
    return 0;
}

#endif
