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
#ifndef __GRAPHICS_SCREENS_H__
#define __GRAPHICS_SCREENS_H__

#include "util/tp_types.h"
#include "graphics/canvas.h"
#include "drivers/resources.h"
#ifdef CONFIG_ILI93XX
#include "drivers/graphics/ili93xx_transport.h"
#endif
#ifdef CONFIG_ILI93XX_BITBANG
#include "drivers/graphics/ili93xx_bitbang.h"
#endif

#ifdef CONFIG_SPI
typedef struct {
    resource_t rst;
    resource_t cs;
    resource_t cd;
    resource_t spi_port;
    resource_t backlight;
} spi_graphics_screen_params_t;
#endif

#ifdef CONFIG_DOGS102X6
typedef spi_graphics_screen_params_t dogs102x6_params_t;
#endif
#ifdef CONFIG_PCD8544
typedef spi_graphics_screen_params_t pcd8544_params_t;
#endif
#ifdef CONFIG_SSD1329
typedef spi_graphics_screen_params_t ssd1329_params_t;
#endif

#ifdef CONFIG_ILI93XX
typedef struct {
    resource_t rst;
    resource_t backlight;
    const ili93xx_db_transport_t *trns;
} ili93xx_params_t;
#endif

#ifdef CONFIG_SDL_SCREEN
typedef struct {
    u16 width;
    u16 height;
} sdl_screen_params_t;
#endif

#ifdef CONFIG_SSD1306
typedef struct {
    resource_t i2c_port;
    resource_t i2c_addr;
} ssd1306_params_t;
#endif

#ifdef CONFIG_DOGS102X6
canvas_t *dogs102x6_new(const dogs102x6_params_t *params);
#endif

#ifdef CONFIG_ILI93XX
canvas_t *ili93xx_new(const ili93xx_params_t *params);
#endif

#ifdef CONFIG_PCD8544
canvas_t *pcd8544_new(const pcd8544_params_t *params);
#endif

#ifdef CONFIG_SDL_SCREEN
canvas_t *sdl_screen_new(const sdl_screen_params_t *params);
#endif

#ifdef CONFIG_SSD1306
canvas_t *ssd1306_new(const ssd1306_params_t *params);
#endif

#ifdef CONFIG_SSD1329
canvas_t *ssd1329_new(const ssd1329_params_t *params);
#endif

#endif
