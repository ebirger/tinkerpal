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
#ifndef __BOARD_H__
#define __BOARD_H__

#include "drivers/resources.h"
#include "platform/platform.h"
#include "drivers/serial/serial.h"
#ifdef CONFIG_GPIO
#include "drivers/gpio/gpio.h"
#endif
#ifdef CONFIG_SPI
#include "drivers/spi/spi.h"
#endif
#ifdef CONFIG_I2C
#include "drivers/i2c/i2c.h"
#endif
#ifdef CONFIG_MMC
#include "drivers/mmc/mmc.h"
#endif
#ifdef CONFIG_ENC28J60
#include "drivers/net/enc28j60.h"
#endif
#ifdef CONFIG_NET_ESP8266
#include "drivers/net/esp8266.h"
#endif
#ifdef CONFIG_GRAPHICS_SCREENS
#include "drivers/graphics/graphics_screens.h"
#endif

typedef struct {
    char *desc;
    resource_t default_console_id;
    const resource_t *leds; /* 0 terminated */
#ifdef CONFIG_ILI93XX
    ili93xx_params_t ili93xx_params;
#endif
#ifdef CONFIG_SDL_SCREEN
    sdl_screen_params_t sdl_screen_params;
#endif
#ifdef CONFIG_DOGS102X6
    dogs102x6_params_t dogs102x6_params;
#endif
#ifdef CONFIG_SSD1306
    ssd1306_params_t ssd1306_params;
#endif
#ifdef CONFIG_SSD1329
    ssd1329_params_t ssd1329_params;
#endif
#ifdef CONFIG_PCD8544
    pcd8544_params_t pcd8544_params;
#endif
#ifdef CONFIG_MMC
    mmc_params_t mmc_params;
#endif
#ifdef CONFIG_ENC28J60
    enc28j60_params_t enc28j60_params;
#endif
#ifdef CONFIG_NET_ESP8266
    esp8266_params_t esp8266_params;
#endif
} board_t;

/* Boards are defined in board_*.c files */
extern const board_t board;

#endif
