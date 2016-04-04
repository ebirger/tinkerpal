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

#ifdef CONFIG_RDK_IDM
#define BOARD_FILE "boards/rdk_idm.board"
#endif
#ifdef CONFIG_EK_LM3S6965
#define BOARD_FILE "boards/ek_lm3s6965.board"
#endif
#ifdef CONFIG_EK_LM4F120XL
#define BOARD_FILE "boards/ek_lm4f120xl.board"
#endif
#ifdef CONFIG_EK_TM4C123GXL
#define BOARD_FILE "boards/ek_tm4c123gxl.board"
#endif
#ifdef CONFIG_EK_TM4C1294XL
#define BOARD_FILE "boards/ek_tm4c1294xl.board"
#endif
#ifdef CONFIG_CC3200
#define BOARD_FILE "boards/cc3200_launchxl.board"
#endif
#ifdef CONFIG_STM32F3DISCOVERY
#define BOARD_FILE "boards/stm32f3discovery.board"
#endif
#ifdef CONFIG_STM32F4DISCOVERY
#define BOARD_FILE "boards/stm32f4discovery.board"
#endif
#ifdef CONFIG_STM32F429IDISCOVERY
#define BOARD_FILE "boards/stm32f429idiscovery.board"
#endif
#ifdef CONFIG_STM32_ARMJISHU_28
#define BOARD_FILE "boards/stm32_armjishu_28.board"
#endif
#ifdef CONFIG_STM32_HY_24
#define BOARD_FILE "boards/stm32_hy_24.board"
#endif
#ifdef CONFIG_FRDM_KL25Z
#define BOARD_FILE "boards/frdm_kl25z.board"
#endif
#ifdef CONFIG_MSP430F5529_EXP
#define BOARD_FILE "boards/430f5529.board"
#endif
#ifdef CONFIG_MSP430F5529_LAUNCHPAD
#define BOARD_FILE "boards/msp430f5529.board"
#endif
#ifdef CONFIG_PLATFORM_EMULATION
#define BOARD_FILE "boards/unix_sim.board"
#endif
#ifdef CONFIG_X86_PLATFORM_EMULATION
#define BOARD_FILE "boards/x86_sim.board"
#endif
#ifdef CONFIG_TRINKET_PRO
#define BOARD_FILE "boards/trinket_pro.board"
#endif
#ifdef CONFIG_ESP8266_512KB
#define BOARD_FILE "boards/esp8266.board"
#endif

#ifdef CONFIG_ILI93XX
extern ili93xx_db_transport_t stm32_fsmc_ili93xx_trns;
#endif

const board_t board = {
#define BOARD_START(_desc) .desc = _desc,

#define DEFAULT_CONSOLE(uart) .default_console_id = uart,

#ifdef CONFIG_SSD1306
#define SSD1306_PARAMS(_i2c_port, _i2c_addr) \
    .ssd1306_params = { \
        .i2c_port = _i2c_port, \
	.i2c_addr = _i2c_addr, \
    },
#endif

#ifdef CONFIG_MMC
#define MMC_PARAMS(_spi_port, _mosi, _cs) \
    .mmc_params = { \
        .spi_port = _spi_port, \
        .mosi = _mosi, \
        .cs = _cs, \
    },
#endif

#ifdef CONFIG_ENC28J60
#define ENC28J60_PARAMS(_spi_port, _cs, _intr) \
    .enc28j60_params = { \
        .spi_port = _spi_port, \
        .cs = _cs, \
        .intr = _intr \
    },
#endif

#ifdef CONFIG_NET_ESP8266
#define ESP8266_PARAMS(_serial_port) \
    .esp8266_params = { \
        .serial_port = _serial_port, \
    },
#endif

#ifdef CONFIG_PCD8544
#define PCD8544_PARAMS(_rst, _cs, _cd, _spi_port, _backlight) \
    .pcd8544_params = { \
        .rst = _rst, \
        .cs = _cs, \
        .cd = _cd, \
        .spi_port = _spi_port, \
        .backlight = _backlight, \
    },
#endif

#ifdef CONFIG_ST7735
#define ST7735_PARAMS(_rst, _cs, _cd, _spi_port, _backlight) \
    .st7735_params = { \
        .rst = _rst, \
        .cs = _cs, \
        .cd = _cd, \
        .spi_port = _spi_port, \
        .backlight = _backlight, \
    },
#endif

#ifdef CONFIG_DOGS102X6
#define DOGS102X6_PARAMS(_rst, _cs, _cd, _spi_port, _backlight) \
    .dogs102x6_params = { \
        .rst = _rst, \
        .cs = _cs, \
        .cd = _cd, \
        .spi_port = _spi_port, \
        .backlight = _backlight, \
    },
#endif

#ifdef CONFIG_SSD1329
#define SSD1329_PARAMS(_rst, _cs, _cd, _spi_port) \
    .ssd1329_params = { \
        .rst = _rst, \
        .cs = _cs, \
        .cd = _cd, \
        .spi_port = _spi_port, \
    },
#endif

#ifdef CONFIG_ST7920
#ifdef CONFIG_SPI
#define ST7920_PRARMS(_mode, _rst, _psb, _rs, _rw, _en, d0, d1, d2, d3, d4, d5, d6, d7, _spi_port, _cs) \
    .st7920_params = { \
        .mode = _mode, \
        .rst = _rst, \
        .psb = _psb, \
        .rs = _rs, \
        .rw = _rw, \
        .en = _en, \
        .spi_port = _spi_port, \
        .cs = _cs, \
    },
#else
#define ST7920_PRARMS(_mode, _rst, _psb, _rs, _rw, _en, d0, d1, d2, d3, d4, d5, d6, d7, _spi_port, _cs) \
    .st7920_params = { \
        .mode = _mode, \
        .rst = _rst, \
        .psb = _psb, \
        .rs = _rs, \
        .rw = _rw, \
        .en = _en, \
        .d = { \
            [0] = d0, \
            [1] = d1, \
            [2] = d2, \
            [3] = d3, \
            [4] = d4, \
            [5] = d5, \
            [6] = d6, \
            [7] = d7 \
        }, \
    },
#endif
#endif

#ifdef CONFIG_ILI93XX
#define ILI93XX_PARAMS(_rst, _backlight, _trns) \
    .ili93xx_params = { \
        .rst = _rst, \
        .backlight = _backlight, \
        .trns = _trns, \
    },
#endif

#ifdef CONFIG_SDL_SCREEN
#define SDL_SCREEN_PARAMS(_width, _height) \
    .sdl_screen_params = { \
        .width = _width, \
        .height = _height, \
    },
#endif

#define BOARD_END(...)

#include "boards/board_cfg.h"

#define LED(l) l,
    .leds = (resource_t []){
#include "boards/board_cfg.h"
        0
    },
};
