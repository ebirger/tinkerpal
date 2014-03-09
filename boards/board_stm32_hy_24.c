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
#include "boards/board.h"
#include "platform/platform.h"

/* LCD daughter board connections:
 *
 * LEDK:GND  | MISO:PA6
 * LEDA:5V   | SCK:PA5
 * NC:PB11   | MOSI:PA7
 * RST:PE1   | CS:PD6
 * NC:PB10   | NC:PC4
 * CS:PD7    | NC:PC5
 * DB17:PD10 | DB7:PE10
 * DB16:PD9  | DB6:PE9
 * DB15:PD8  | DB5:PE8
 * DB14:PE15 | DB4:PE7
 * DB13:PE14 | DB3:PD1
 * DB12:PE13 | DB2:PD0
 * DB11:PE12 | DB1:PD15
 * DB10:PE11 | DB0:PD14
 * RD:PD4    | IRQ:PB6
 * RW:PD5    | DOUT:PA6
 * RS:PD11   | BUSY:PC13
 * NC:PC6    | DIN:PA7
 * VCC:3V    | CS:PB7
 * GND:GND   | DCLK:PA5
 */
extern ili93xx_db_transport_t stm32_fsmc_ili93xx_trns;

static const resource_t leds[] = {
    GPIO_RES(PC6),
    GPIO_RES(PC7),
    GPIO_RES(PD13),
    GPIO_RES(PD6),
    0
};

const board_t board = {
    .desc = "HY 24'' STM32F103VET6 based board",
    .default_console_id = UART_RES(USART_PORT2),
    .leds = leds,
#ifdef CONFIG_ILI93XX
    .ili93xx_params = {
	.trns = &stm32_fsmc_ili93xx_trns,
	.rst = GPIO_RES(PE1),
	.backlight = GPIO_RES(PB10),
    },
#endif
};
