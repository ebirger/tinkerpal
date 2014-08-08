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
#include "drivers/graphics/ili93xx_controllers.h"

const ili93xx_cmd_t ili9325_4532_init_cmds[] = {
    REG_SET(0x00, 0x0001),

    DELAY_MS(10),

    REG_SET(0x15, 0x0030),
    REG_SET(0x11, 0x0040),
    REG_SET(0x10, 0x1628),
    REG_SET(0x12, 0x0000),
    REG_SET(0x13, 0x104d),

    DELAY_MS(10),

    REG_SET(0x12, 0x0010),

    DELAY_MS(10),

    REG_SET(0x10, 0x2620),
    REG_SET(0x13, 0x344d),

    DELAY_MS(10),

    REG_SET(0x01, 0x0100),
    REG_SET(0x02, 0x0300),
    REG_SET(0x03, 0x0030),
    REG_SET(0x08, 0x0604),
    REG_SET(0x09, 0x0000),
    REG_SET(0x0A, 0x0008),

    REG_SET(0x41, 0x0002),
    REG_SET(0x60, 0x2700),
    REG_SET(0x61, 0x0001),
    REG_SET(0x90, 0x0182),
    REG_SET(0x93, 0x0001),
    REG_SET(0xa3, 0x0010),

    DELAY_MS(10),

    /* Gamma Curve */
    REG_SET(0x30, 0x0000),
    REG_SET(0x31, 0x0502),
    REG_SET(0x32, 0x0307),
    REG_SET(0x33, 0x0305),
    REG_SET(0x34, 0x0004),
    REG_SET(0x35, 0x0402),
    REG_SET(0x36, 0x0707),
    REG_SET(0x37, 0x0503),
    REG_SET(0x38, 0x1505),
    REG_SET(0x39, 0x1505),
    DELAY_MS(10),

    /* Display On */
    REG_SET(0x07, 0x0001),
    DELAY_MS(10),
    REG_SET(0x07, 0x0021),
    REG_SET(0x07, 0x0023),
    DELAY_MS(10),
    REG_SET(0x07, 0x0033),
    DELAY_MS(10),
    REG_SET(0x07, 0x0133),

    SEQUENCE_END
};
