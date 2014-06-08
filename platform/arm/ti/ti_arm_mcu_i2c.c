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
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "platform/platform.h"
#include "platform/arm/ti/ti_arm_mcu.h"

static inline void ti_arm_mcu_pin_mode_i2c(int pin, int scl, int i2c_af)
{
    ti_arm_mcu_periph_enable(ti_arm_mcu_gpio_periph(pin));
    if (i2c_af)
        MAP_GPIOPinConfigure(i2c_af);
    MAP_GPIOPinTypeI2C(ti_arm_mcu_gpio_base(pin), GPIO_BIT(pin));
    if (scl)
	ti_arm_mcu_pin_config(pin, GPIO_PIN_TYPE_STD_WPU);
    else
	ti_arm_mcu_pin_config(pin, GPIO_PIN_TYPE_OD);
}

int ti_arm_mcu_i2c_init(int port)
{
    const ti_arm_mcu_i2c_t *i2c = &ti_arm_mcu_i2cs[port];

    ti_arm_mcu_periph_enable(i2c->periph);
    MAP_I2CMasterInitExpClk(i2c->base, platform.get_system_clock(), 0);
    ti_arm_mcu_pin_mode_i2c(i2c->scl, 1, i2c->scl_af);
    ti_arm_mcu_pin_mode_i2c(i2c->sda, 0, i2c->sda_af);
    return 0;
}

static void wait_for_completion(unsigned long base)
{
#ifdef CONFIG_TI_I2C_WAIT_FOR_BUSY_WAR
    /* XXX: Ugly workaround:
     * Busy flag takes a while to be asserted, so we wait for it.
     * Assumption is that the operation is long enough so we won't
     * miss it
     */
    while (!MAP_I2CMasterBusy(base));
#endif
    while (MAP_I2CMasterBusy(base));
}

void ti_arm_mcu_i2c_reg_write(int port, unsigned char addr, unsigned char reg,
    unsigned char *data, int len)
{
    const ti_arm_mcu_i2c_t *i2c = &ti_arm_mcu_i2cs[port];
    
    MAP_I2CMasterSlaveAddrSet(i2c->base, addr >> 1, 0);
    MAP_I2CMasterDataPut(i2c->base, reg);
    MAP_I2CMasterControl(i2c->base, I2C_MASTER_CMD_BURST_SEND_START);
    wait_for_completion(i2c->base);
    while (len--)
    {
        MAP_I2CMasterDataPut(i2c->base, *data++);
	if (len)
	{
	    MAP_I2CMasterControl(i2c->base, I2C_MASTER_CMD_BURST_SEND_CONT);
	    wait_for_completion(i2c->base);
	}
    }
    MAP_I2CMasterControl(i2c->base, I2C_MASTER_CMD_BURST_SEND_FINISH);
    wait_for_completion(i2c->base);
}
