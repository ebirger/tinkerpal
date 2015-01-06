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
    GPIO_RES(PF1),
    GPIO_RES(PF2),
    GPIO_RES(PF3),
    0
};

const board_t board = {
    .desc = "EK TM4C123GXL (Tiva C Launchpad)",
    .default_console_id = UART_RES(UART0),
    .leds = leds,
#ifdef CONFIG_SSD1306
    .ssd1306_params = {
        .i2c_port = I2C_RES(I2C1),
	.i2c_addr = 0x78,
    },
#endif
#ifdef CONFIG_MMC
    .mmc_params = {
        .spi_port = SPI_RES(SSI0),
        .mosi = GPIO_RES(PA5),
        .cs = GPIO_RES(PB6),
    },
#endif
#ifdef CONFIG_ENC28J60
    .enc28j60_params = {
        .spi_port = SPI_RES(SSI1),
        .cs = GPIO_RES(PE3),
        .intr = GPIO_RES(PF4),
    },
#endif
#ifdef CONFIG_NET_ESP8266
    .esp8266_params = {
        .serial_port = UART_RES(UART4),
    },
#endif
#ifdef CONFIG_PCD8544
    .pcd8544_params = {
        .rst = GPIO_RES(PF3),
        .cs = GPIO_RES(PB6),
        .cd = GPIO_RES(PB4),
        .spi_port = SPI_RES(SSI0),
        .backlight = GPIO_RES(PF2),
    },
#endif
#ifdef CONFIG_ST7735
    .st7735_params = {
        .rst = GPIO_RES(PF3),
        .cs = GPIO_RES(PB6),
        .cd = GPIO_RES(PA6),
        .spi_port = SPI_RES(SSI0),
        .backlight = GPIO_RES(PF2),
    },
#endif
#ifdef CONFIG_ST7920
    .st7920_params = {
        .rs = GPIO_RES(PE3),
        .rw = GPIO_RES(PA7),
        .en = GPIO_RES(PE1),
        .rst = GPIO_RES(PB1),
        .psb = GPIO_RES(PB0),
        .d = {
            [0] = GPIO_RES(PF2),
            [1] = GPIO_RES(PD6),
            [2] = GPIO_RES(PA2),
            [3] = GPIO_RES(PA3),
            [4] = GPIO_RES(PD3),
            [5] = GPIO_RES(PD2),
            [6] = GPIO_RES(PD1),
            [7] = GPIO_RES(PD0)
        },
    },
#endif
};
