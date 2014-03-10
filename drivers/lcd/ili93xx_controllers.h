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
#ifndef __ILI93XX_CONTROLLERS_H__
#define __ILI93XX_CONTROLLERS_H__

#include "util/tp_types.h"

typedef struct {
#define CMD_END ((u16)-1)
#define CMD_DELAY ((u16)-2)
    u16 cmd; /* Register number or command */
    u16 data; /* Register data or delay in ms */
} ili93xx_cmd_t;

#define REG_SET(c, d) { .cmd = c, .data = d }
#define DELAY_MS(ms) { .cmd = CMD_DELAY, .data = ms }
#define SEQUENCE_END { .cmd = CMD_END }

#ifdef CONFIG_ILI9328
extern const ili93xx_cmd_t ili9328_init_cmds[];
#endif
#ifdef CONFIG_ILI9325
extern const ili93xx_cmd_t ili9325_4532_init_cmds[];
#endif
#ifdef CONFIG_ILI9320
extern const ili93xx_cmd_t ili9320_init_cmds[];
#endif

#endif
