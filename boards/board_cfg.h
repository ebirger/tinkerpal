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
#ifndef BOARD_START
#define BOARD_START(...)
#endif

#ifndef DEFAULT_CONSOLE
#define DEFAULT_CONSOLE(...)
#endif

#ifndef LED
#define LED(...)
#endif

#ifndef SSD1306_PARAMS
#define SSD1306_PARAMS(...)
#endif

#ifndef MMC_PARAMS
#define MMC_PARAMS(...)
#endif

#ifndef ENC28J60_PARAMS
#define ENC28J60_PARAMS(...)
#endif

#ifndef ESP8266_PARAMS
#define ESP8266_PARAMS(...)
#endif

#ifndef PCD8544_PARAMS
#define PCD8544_PARAMS(...)
#endif

#ifndef ST7735_PARAMS
#define ST7735_PARAMS(...)
#endif

#ifndef DOGS102X6_PARAMS
#define DOGS102X6_PARAMS(...)
#endif

#ifndef SSD1329_PARAMS
#define SSD1329_PARAMS(...)
#endif

#ifndef ST7920_PRARMS
#define ST7920_PRARMS(...)
#endif

#ifndef ILI93XX_PARAMS
#define ILI93XX_PARAMS(...)
#endif

#ifndef SDL_SCREEN_PARAMS
#define SDL_SCREEN_PARAMS(...)
#endif

#ifndef BOARD_END
#define BOARD_END(...)
#endif

#include BOARD_FILE

#undef BOARD_START
#undef DEFAULT_CONSOLE
#undef LED
#undef SSD1306_PARAMS
#undef MMC_PARAMS
#undef ENC28J60_PARAMS
#undef ESP8266_PARAMS
#undef PCD8544_PARAMS
#undef ST7735_PARAMS
#undef DOGS102X6_PARAMS
#undef SSD1306_PARAMS
#undef SSD1329_PARAMS
#undef ST7920_PRARMS
#undef ILI93XX_PARAMS
#undef SDL_SCREEN_PARAMS
#undef BOARD_END
