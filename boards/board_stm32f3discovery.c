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

static const resource_t leds[] = {
    GPIO_RES(PE8),
    GPIO_RES(PE9),
    GPIO_RES(PE10),
    GPIO_RES(PE11),
    GPIO_RES(PE12),
    GPIO_RES(PE13),
    GPIO_RES(PE14),
    GPIO_RES(PE15),
    0
};

const board_t board = {
    .desc = "STM32F3Discovery",
    .default_console_id = UART_RES(USART_PORT2),
    .leds = leds,
#ifdef CONFIG_ILI93XX
    .ili93xx_params = {
        .trns = ILI93XX_BITBANG_TRNS({
            .rs = GPIO_RES(PB15),
            .wr = GPIO_RES(PC7),
            .rd = GPIO_RES(PC6),
            .data_port_low = GPIO_RES(GPIO_PORT_D),
            .data_port_high = GPIO_RES(GPIO_PORT_D),
            .data_port_high_shift = 8,
        }),
        .rst = GPIO_RES(PB1),
        .backlight = GPIO_RES(PC0),
    },
#endif
};
