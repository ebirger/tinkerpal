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
#include "drivers/resources.h"
#include "drivers/lcd/ili93xx.h"
#include "drivers/lcd/ili93xx_controllers.h"

#define RST(i) ((i)->params.rst)
#define RS(i) ((i)->params.rs)
#define RD(i) ((i)->params.rd)
#define WR(i) ((i)->params.wr)
#define BL(i) ((i)->params.backlight)
#define DH(i) ((i)->params.data_port_high)
#define DL(i) ((i)->params.data_port_low)

typedef struct {
    canvas_t canvas;
    ili93xx_params_t params;
} ili93xx_t;

#define ILI93XX_FROM_CANVAS(c) container_of(c, ili93xx_t, canvas);

static ili93xx_t g_ili93xx;

static u16 ili93xx_read_data(ili93xx_t *i)
{
    u16 ret;

    gpio_set_port_mode(DL(i), GPIO_PM_INPUT);
    gpio_set_port_mode(DH(i), GPIO_PM_INPUT);

    gpio_digital_write(RD(i), 0);

    ret = gpio_get_port_val(DH(i)) << 8;
    ret |= gpio_get_port_val(DL(i));

    gpio_digital_write(RD(i), 1);

    gpio_set_port_mode(DL(i), GPIO_PM_OUTPUT);
    gpio_set_port_mode(DH(i), GPIO_PM_OUTPUT);

    return ret;
}

static void ili93xx_write_data(ili93xx_t *i, u16 data)
{
    gpio_set_port_val(DH(i), (data >> 8) & 0xff);
    gpio_set_port_val(DL(i), data & 0xff);

    gpio_digital_write(WR(i), 0);
    gpio_digital_write(WR(i), 1);
}

static void ili93xx_write_cmd(ili93xx_t *i, u8 cmd)
{
    gpio_set_port_val(DH(i), 0);
    gpio_set_port_val(DL(i), cmd);

    gpio_digital_write(RS(i), 0);
    gpio_digital_write(WR(i), 0);
    gpio_digital_write(WR(i), 1);
    gpio_digital_write(RS(i), 1);
}

static u16 ili93xx_reg_read(ili93xx_t *i, u8 reg)
{
    ili93xx_write_cmd(i, reg);
    return ili93xx_read_data(i);
}

static void ili93xx_reg_write(ili93xx_t *i, u8 reg, u16 data)
{
    ili93xx_write_cmd(i, reg);
    ili93xx_write_data(i, data);
}

static int chip_init(ili93xx_t *i)
{
    const ili93xx_cmd_t *sequence = NULL;
    u16 chip_id;

    /* Set GPIO Modes */
    if (gpio_set_pin_mode(RST(i), GPIO_PM_OUTPUT) ||
        gpio_set_pin_mode(BL(i), GPIO_PM_OUTPUT) ||
        gpio_set_pin_mode(RS(i), GPIO_PM_OUTPUT) ||
        gpio_set_pin_mode(WR(i), GPIO_PM_OUTPUT) ||
        gpio_set_pin_mode(RD(i), GPIO_PM_OUTPUT))
    {
	tp_err(("Unable to set pin mode for control pins\n"));
	return -1;
    }

    if (gpio_set_port_mode(DL(i), GPIO_PM_OUTPUT) ||
	gpio_set_port_mode(DH(i), GPIO_PM_OUTPUT))
    {
	tp_err(("Unable to set pin mode for data ports\n"));
	return -1;
    }

    /* Set GPIO Values */
    gpio_set_port_val(DL(i), 0);
    gpio_set_port_val(DH(i), 0);
    gpio_digital_write(BL(i), 0);
    gpio_digital_write(WR(i), 1);
    gpio_digital_write(RD(i), 1);
    gpio_digital_write(RS(i), 1);

    /* Reset */
    gpio_digital_write(RST(i), 0);

    platform_msleep(10);

    gpio_digital_write(RST(i), 1);

    /* Wait for reset to complete */
    platform_msleep(60);

    chip_id = ili93xx_reg_read(i, 0x00);

    tp_out(("Found ILI93xx controller. Chip ID: %x\n", chip_id));

    /* Process init sequence */
#ifdef CONFIG_ILI9328
    if (chip_id == 0x9328)
	sequence = ili9328_init_cmds;
#endif

    if (!sequence)
    {
	tp_err(("Unsupported ILI93xx controller\n"));
	return -1;
    }

    for (; sequence->cmd != CMD_END; sequence++)
    {
	if (sequence->cmd == CMD_DELAY)
	    platform_msleep(sequence->data);
	else
	    ili93xx_reg_write(i, (u8)sequence->cmd, sequence->data);
    }
    return 0;
}

void ili93xx_pixel_set(canvas_t *c, u16 x, u16 y, u16 val)
{
    ili93xx_t *i = ILI93XX_FROM_CANVAS(c);

    /* X value */
    ili93xx_write_cmd(i, 0x20);
    ili93xx_write_data(i, x);

    /* Y value */
    ili93xx_write_cmd(i, 0x21);
    ili93xx_write_data(i, y);

    /* Pixel value */
    ili93xx_write_cmd(i, 0x22);
    ili93xx_write_data(i, val);
}

static const canvas_ops_t ili93xx_ops = {
    .pixel_set = ili93xx_pixel_set,
};

canvas_t *ili93xx_new(ili93xx_params_t *params)
{
    ili93xx_t *i = &g_ili93xx;

    i->canvas.width = 240;
    i->canvas.height = 320;
    i->canvas.ops = &ili93xx_ops;

    i->params = *params;
    chip_init(i);

    gpio_digital_write(BL(i), 1);
    return &i->canvas;
}