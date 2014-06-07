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
#include "util/tp_misc.h"
#include "util/event.h"
#include "util/debug.h"
#include "drivers/resources.h"
#include "drivers/lcd/ssd1306.h"
#include "drivers/gpio/gpio.h"
#include "drivers/i2c/i2c.h"
#include "graphics/colors.h"

typedef struct {
    ssd1306_params_t params;
    canvas_t canvas;
} ssd1306_t;

#define SSD1306_DISPLAY_OFF 0xae
#define SSD1306_DISPLAY_ON 0xaf
#define SSD1306_SET_DISPLAY_CLOCK_DIV 0xd5
#define SSD1306_SET_MULTIPLEX_RATIO 0xa8
#define SSD1306_SET_DISPLAY_OFFSET 0xd3
#define SSD1306_SET_START_LINE 0x40
#define SSD1306_CHARGE_PUMP 0x8d
#define SSD1306_MEM_ADDR_MODE 0x20
#define SSD1306_SEGMENT_RE_MAP 0xa0
#define SSD1306_SET_COM_OUTPUT_SCAN_DIR(dec) (0xc0 | ((dec) ? 0x08 : 0x0))
#define SSD1306_SET_COM_PINS 0xda
#define SSD1306_SET_CONTRAST 0x81
#define SSD1306_SET_PRE_CHARGE 0xd9
#define SSD1306_SET_VCOM_DSELECT 0xdb
#define SSD1306_ENTIRE_DISPLAY_ON_RESUME 0xa4
#define SSD1306_SET_NORMAL_DISPLAY 0xa6
#define SSD1306_INIT_SEQ_END 0xff

static u8 ssd1306_init_seq[] = {
    SSD1306_DISPLAY_OFF,
    SSD1306_SET_MULTIPLEX_RATIO,
    63,
    SSD1306_SET_DISPLAY_OFFSET,
    0x0,
    SSD1306_SET_START_LINE | 0x0,
    SSD1306_SEGMENT_RE_MAP | 0x1,
    SSD1306_SET_COM_OUTPUT_SCAN_DIR(1),
    SSD1306_SET_COM_PINS,
    0x12,
    SSD1306_SET_CONTRAST,
    0xcf,
    SSD1306_ENTIRE_DISPLAY_ON_RESUME,
    SSD1306_SET_NORMAL_DISPLAY,
    SSD1306_SET_DISPLAY_CLOCK_DIV,
    0x80, /* Default value per spec */
    SSD1306_CHARGE_PUMP,
    0x14,
    SSD1306_SET_PRE_CHARGE,
    0xf1,
    SSD1306_SET_VCOM_DSELECT,
    0x40,
    SSD1306_MEM_ADDR_MODE,
    0x00,
    SSD1306_DISPLAY_ON,
    SSD1306_INIT_SEQ_END
};

static ssd1306_t g_ssd1306_screen;

static void chip_init(ssd1306_t *screen)
{
    u8 *cmd;

    tp_out(("SSD1306 chip init!\n"));
    i2c_init(screen->params.i2c_port);
    
    for (cmd = ssd1306_init_seq; *cmd != SSD1306_INIT_SEQ_END; cmd++)
    {
	i2c_reg_write(screen->params.i2c_port, screen->params.i2c_addr, 0, cmd,
	    1);
    }
}

static void ssd1306_pixel_set(canvas_t *c, u16 x, u16 y, u16 val)
{
}

static const canvas_ops_t ssd1306_ops = {
    .pixel_set = ssd1306_pixel_set,
};

canvas_t *ssd1306_new(const ssd1306_params_t *params)
{
    ssd1306_t *screen = &g_ssd1306_screen;

    screen->params = *params;

    chip_init(screen);

    screen->canvas.width = 128;
    screen->canvas.height = 64;
    screen->canvas.ops = &ssd1306_ops;

    canvas_register(&screen->canvas);
    return &screen->canvas;
}
