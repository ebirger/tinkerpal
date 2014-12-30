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
#ifndef __DRIVERS_I2C_H__
#define __DRIVERS_I2C_H__

#include "drivers/resources.h"
#include "platform/platform.h"
#include "util/tstr.h"

#define I2C_RES(port) RES(I2C_RESOURCE_ID_BASE, port, 0)

static inline int i2c_init(resource_t port)
{
    if (RES_BASE(port) != I2C_RESOURCE_ID_BASE)
        return -1;

    return platform.i2c.init(RES_MAJ(port));
}

static inline void i2c_reg_write(int port, u8 addr, u8 reg, const u8 *data,
    int len)
{
    if (RES_BASE(port) != I2C_RESOURCE_ID_BASE)
        return;

    return platform.i2c.reg_write(RES_MAJ(port), addr, reg, data, len);
}

static inline int i2c_get_constant(int *constant, tstr_t t)
{
    int pfx_len;

#define I2C_PREFIX "I2C"
    pfx_len = sizeof(I2C_PREFIX) - 1;

    if (tstr_ncmp_str(&t, I2C_PREFIX, pfx_len))
        return -1;

    if (t.len != pfx_len + 1)
        return -1;

    *constant = (int)RES(I2C_RESOURCE_ID_BASE, tstr_peek(&t, pfx_len) - '0', 0);
    return 0;
}

#endif
