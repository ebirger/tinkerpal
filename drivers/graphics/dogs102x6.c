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
#include "drivers/graphics/graphics_screens.h"
#include "drivers/gpio/gpio.h"
#include "drivers/spi/spi.h"
#include "graphics/colors.h"

#define SET_COLUMN_ADDRESS_LSB 0x00 /* [0..3] = CA[0..3] */
#define SET_COLUMN_ADDRESS_MSB 0x10 /* [0..3] = CA[4..7] */
#define SET_POWER_CONTROL 0x28
#define POWER_CTRL_BOOSTER_ON 0x01
#define POWER_CTRL_REGULATOR_ON 0x02
#define POWER_CTRL_FOLLOWER_ON 0x04
#define SET_SCROLL_LINE 0x40 /* [0..5] = Start Line */
#define SET_PAGE_ADDRESS 0xb0 /* [0..3] = PA 0-7 */
#define SET_VLCD_RESISTOR_RATIO 0x20 /* [0..2] = PC[3..5] range 0-7 */
#define SET_ELECTRONIC_VOLUME_CMD 0x81 /* Adjusts contrast - val in next byte */
#define SET_ALL_PIXEL_ON 0xa4 /* bit 0 - '0' sram content, '1' all pix on */
#define SET_INVERSE_DISPLAY 0xa6 /* bit 0 - '1' invert display */
#define SET_DISPLAY_ENABLE 0xae /* bit 0 - '0' sleep, '1' on */
#define SET_SEG_DIRECTION 0xa0 /* bit 0 - '0' 0..131 ,'1' mirror 131..0 */
#define SET_COM_DIRECTION 0xc0 /* bit 3 - '0' 0..63 ,'1' mirror 63..0 */
#define SET_LCD_BIAS_RATIO 0xa2 /* bit 0 - '0' 1/9, '1' 1/7 */
#define SET_ADV_PROGRAM_CONTROL0_CMD 0xfa /* Adv features - val in next byte */
#define SET_ADV_PROGRAM_CONTROL0_OPTION 0x10
#define ADV_CTRL0_TEMP_COMP_BIT 0x80 /* '0' -0.05, '1' -0,11%/°C */
#define ADV_CTRL0_WC_BIT 0x02 /* Column wrap around. '0' off, '1' on */
#define ADV_CTRL0_WP_BIT 0x01 /* Page wrap around. '0' off, '1' on */

typedef struct {
    dogs102x6_params_t params;
    canvas_t canvas;
    u8 shadow[102 * 8];
} dogs102x6_t;

static dogs102x6_t g_dogs102x6_screen;

static u8 dogs102x6_init_seq[] = {
    SET_SCROLL_LINE,
    SET_SEG_DIRECTION | 0x01, /* Mirror display - should be configurable */
    SET_COM_DIRECTION | 0x08, /* Mirror display - should be configurable */
    SET_ALL_PIXEL_ON,
    SET_INVERSE_DISPLAY,
    SET_LCD_BIAS_RATIO,
    SET_POWER_CONTROL | POWER_CTRL_BOOSTER_ON | POWER_CTRL_REGULATOR_ON |
        POWER_CTRL_FOLLOWER_ON,
    SET_VLCD_RESISTOR_RATIO | 0x07,
    SET_ELECTRONIC_VOLUME_CMD,
    11, /* Initial contrast */
    SET_ADV_PROGRAM_CONTROL0_CMD,
    SET_ADV_PROGRAM_CONTROL0_OPTION | ADV_CTRL0_TEMP_COMP_BIT,
    SET_DISPLAY_ENABLE | 0x01,
    SET_PAGE_ADDRESS,
    SET_COLUMN_ADDRESS_MSB,
    SET_COLUMN_ADDRESS_LSB
};

static void dogs102x6_write(dogs102x6_t *screen, int iscmd, u8 *data, int len)
{
    gpio_digital_write(screen->params.cs, 0);
    gpio_digital_write(screen->params.cd, !iscmd);

    spi_send_mult(screen->params.spi_port, data, len);

    gpio_digital_write(screen->params.cs, 1);
}

static void dogs102x6_set_address(dogs102x6_t *screen, u8 pa, u8 ca)
{
    u8 cmd, col_addr[2];
    
    cmd = SET_PAGE_ADDRESS | (7 - pa); /* Default to mirrored display */
    col_addr[0] = SET_COLUMN_ADDRESS_LSB + (ca & 0x0f);
    col_addr[1] = SET_COLUMN_ADDRESS_MSB + ((ca & 0xf0) >> 4);

    /* Set page address */
    dogs102x6_write(screen, 1, &cmd, 1);
    /* Set column address */
    dogs102x6_write(screen, 1, col_addr, 2);
}

static void chip_init(dogs102x6_t *screen)
{
    gpio_set_pin_mode(screen->params.rst, GPIO_PM_OUTPUT);
    gpio_set_pin_mode(screen->params.cs, GPIO_PM_OUTPUT);
    gpio_set_pin_mode(screen->params.cd, GPIO_PM_OUTPUT);
    gpio_set_pin_mode(screen->params.backlight, GPIO_PM_OUTPUT);

    /* Reset */
    gpio_digital_write(screen->params.rst, 0);
    gpio_digital_write(screen->params.rst, 1);

    gpio_digital_write(screen->params.cd, 0);
    gpio_digital_write(screen->params.backlight, 1);

    /* Init SPI */
    gpio_digital_write(screen->params.cs, 0);

    spi_init(screen->params.spi_port);
    spi_set_max_speed(screen->params.spi_port, 8000000);

    /* Send init sequence */
    dogs102x6_write(screen, 1, dogs102x6_init_seq,
        sizeof(dogs102x6_init_seq));

    gpio_digital_write(screen->params.cs, 1);
}

static void dogs102x6_pixel_set(canvas_t *c, u16 x, u16 y, u16 val)
{
    dogs102x6_t *screen = container_of(c, dogs102x6_t, canvas);
    u8 page, line_bit;
    
    page = y >> 3; /* Each 8 vertical lines are one byte */
    line_bit = 0x80 >> (y & 0x07); /* Default to mirrored display */

    /* Set shadow */
    if (val)
        screen->shadow[page * 102 + x] |= line_bit;
    else
        screen->shadow[page * 102 + x] &= ~line_bit;

    dogs102x6_set_address(screen, page, x);

    /* Draw on screen */
    dogs102x6_write(screen, 0, screen->shadow + (page * 102 + x), 1);
}

static const canvas_ops_t dogs102x6_ops = {
    .pixel_set = dogs102x6_pixel_set,
};

canvas_t *dogs102x6_new(const dogs102x6_params_t *params)
{
    dogs102x6_t *screen = &g_dogs102x6_screen;

    screen->params = *params;

    chip_init(screen);

    screen->canvas.width = 102;
    screen->canvas.height = 64;
    screen->canvas.ops = &dogs102x6_ops;

    canvas_register(&screen->canvas);
    return &screen->canvas;
}
