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
#include "graphics/js_canvas.h"
#include "boards/board.h"
#include "js/js_obj.h"

#define LCD_CONSTRUCTOR(name) \
int do_##name##_constructor(obj_t **ret, obj_t *this, int argc, obj_t *argv[]) \
{ \
    canvas_t *canvas; \
    /* TODO: allow providing pinout */ \
    canvas = name##_new(&board.name##_params); \
    return canvas_obj_constructor(canvas, ret, this, argc, argv); \
}

#ifdef CONFIG_SSD1329
LCD_CONSTRUCTOR(ssd1329)
#endif

#ifdef CONFIG_SSD1306
LCD_CONSTRUCTOR(ssd1306)
#endif

#ifdef CONFIG_SDL_SCREEN
LCD_CONSTRUCTOR(sdl_screen)
#endif

#ifdef CONFIG_PCD8544
LCD_CONSTRUCTOR(pcd8544)
#endif

#ifdef CONFIG_ST7735
LCD_CONSTRUCTOR(st7735)
#endif

#ifdef CONFIG_ST7920
LCD_CONSTRUCTOR(st7920)
#endif

#ifdef CONFIG_ILI93XX
LCD_CONSTRUCTOR(ili93xx)
#endif

#ifdef CONFIG_DOGS102X6
LCD_CONSTRUCTOR(dogs102x6)
#endif
