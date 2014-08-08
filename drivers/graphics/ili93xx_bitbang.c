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
#include "drivers/gpio/gpio.h"
#include "drivers/graphics/ili93xx.h"
#include "drivers/graphics/ili93xx_bitbang.h"

#define RS(t) ((t)->params.rs)
#define RD(t) ((t)->params.rd)
#define WR(t) ((t)->params.wr)
#define DH(t) ((t)->params.data_port_high)
#define DH_SHIFT(t) ((t)->params.data_port_high_shift)
#define DH_MASK(t) (0xff << DH_SHIFT(t))
#define DL(t) ((t)->params.data_port_low)
#define DL_SHIFT(t) ((t)->params.data_port_low_shift)
#define DL_MASK(t) (0xff << DL_SHIFT(t))

static int ili93xx_bitbang_db_init(const ili93xx_db_transport_t *trns)
{
    ili93xx_db_bitbang_t *t = (ili93xx_db_bitbang_t *)trns;

    /* Set GPIO Modes */
    if (gpio_set_pin_mode(RS(t), GPIO_PM_OUTPUT) ||
        gpio_set_pin_mode(WR(t), GPIO_PM_OUTPUT) ||
        gpio_set_pin_mode(RD(t), GPIO_PM_OUTPUT))
    {
        tp_err(("Unable to set pin mode for control pins\n"));
        return -1;
    }

    if (gpio_set_port_mode(DL(t), DL_MASK(t), GPIO_PM_OUTPUT) ||
        gpio_set_port_mode(DH(t), DH_MASK(t), GPIO_PM_OUTPUT))
    {
        tp_err(("Unable to set pin mode for data ports\n"));
        return -1;
    }

    /* Set GPIO Values */
    gpio_digital_write(WR(t), 1);
    gpio_digital_write(RD(t), 1);
    gpio_digital_write(RS(t), 1);

    gpio_set_port_val(DL(t), DL_MASK(t), 0);
    gpio_set_port_val(DH(t), DH_MASK(t), 0);
    return 0;
}

static void ili93xx_bitbang_db_write(ili93xx_db_bitbang_t *t, u16 data)
{
    gpio_set_port_val(DH(t), DH_MASK(t), ((data >> 8) & 0xff) << DH_SHIFT(t));
    gpio_set_port_val(DL(t), DL_MASK(t), (data & 0xff) << DL_SHIFT(t));
}

static void ili93xx_bitbang_db_cmd_wr(const ili93xx_db_transport_t *trns,
    u16 cmd)
{
    ili93xx_db_bitbang_t *t = (ili93xx_db_bitbang_t *)trns;

    ili93xx_bitbang_db_write(t, cmd);

    gpio_digital_write(RS(t), 0);
    gpio_digital_write(WR(t), 0);
    gpio_digital_write(WR(t), 1);
    gpio_digital_write(RS(t), 1);
}

static void ili93xx_bitbang_db_data_wr(const ili93xx_db_transport_t *trns,
    u16 data)
{
    ili93xx_db_bitbang_t *t = (ili93xx_db_bitbang_t *)trns;

    ili93xx_bitbang_db_write(t, data);

    gpio_digital_write(WR(t), 0);
    gpio_digital_write(WR(t), 1);
}

static u16 ili93xx_bitbang_db_data_rd(const ili93xx_db_transport_t *trns)
{
    ili93xx_db_bitbang_t *t = (ili93xx_db_bitbang_t *)trns;
    u16 ret, dh, dl;

    gpio_set_port_mode(DL(t), DL_MASK(t), GPIO_PM_INPUT);
    gpio_set_port_mode(DH(t), DH_MASK(t), GPIO_PM_INPUT);

    gpio_digital_write(RD(t), 0);

    dl = (gpio_get_port_val(DL(t), DL_MASK(t)) >> DL_SHIFT(t)) & 0xff;
    dh = (gpio_get_port_val(DH(t), DH_MASK(t)) >> DH_SHIFT(t)) & 0xff;

    ret = (dh << 8) | dl;

    gpio_digital_write(RD(t), 1);

    gpio_set_port_mode(DL(t), DL_MASK(t), GPIO_PM_OUTPUT);
    gpio_set_port_mode(DH(t), DH_MASK(t), GPIO_PM_OUTPUT);

    return ret;
}

const ili93xx_db_transport_ops_t ili93xx_bitbang_ops = {
    .db_init = ili93xx_bitbang_db_init,
    .db_cmd_wr = ili93xx_bitbang_db_cmd_wr,
    .db_data_wr = ili93xx_bitbang_db_data_wr,
    .db_data_rd = ili93xx_bitbang_db_data_rd,
};
