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

const board_t board = {
    .desc = "RDK-IDM (LM3S6918)",
    .default_console_id = UART_RES(UART1),
#ifdef CONFIG_ILI93XX
    .ili93xx_params = {
        .trns = ILI93XX_BITBANG_TRNS({
            .rs = GPIO_RES(PF2),
            .wr = GPIO_RES(PF1),
            .rd = GPIO_RES(PF0),
            .data_port_low = GPIO_RES(GPIO_PORT_B),
            .data_port_high = GPIO_RES(GPIO_PORT_A),
        }),
        .rst = GPIO_RES(PG0),
        .backlight = GPIO_RES(PC6),
    },
#endif
#ifdef CONFIG_MMC
    .mmc_params = {
        .spi_port = SPI_RES(SSI1),
        .mosi = GPIO_RES(PE3),
        .cs = GPIO_RES(PE1),
    },
#endif
};
