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
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "util/debug.h"
#include "drivers/gpio/gpio_platform.h"
#include "platform/platform.h"
#include "platform/arm/cortex-m.h"
#include "platform/arm/ti/ti_arm_mcu.h"
#include "platform/arm/ti/cc3200/cc3200.h"

const ti_arm_mcu_gpio_port_t ti_arm_mcu_gpio_ports[] = {
    [GPIO_PORT_A] = { PRCM_GPIOA0, GPIOA0_BASE, 0 },
    [GPIO_PORT_B] = { PRCM_GPIOA1, GPIOA1_BASE, 0 },
    [GPIO_PORT_C] = { PRCM_GPIOA2, GPIOA2_BASE, 0 },
    [GPIO_PORT_D] = { PRCM_GPIOA3, GPIOA3_BASE, 0 },
};

const ti_arm_mcu_uart_t ti_arm_mcu_uarts[] = {
};

static unsigned long system_clock;

#ifdef CONFIG_GPIO

static int cc3200_set_pin_mode(int pin, gpio_pin_mode_t mode)
{
    ti_arm_mcu_periph_enable(ti_arm_mcu_gpio_periph(pin));

    switch (mode)
    {
    case GPIO_PM_OUTPUT:
        ti_arm_mcu_pin_mode_output(pin);
        break;
    case GPIO_PM_INPUT:
    case GPIO_PM_INPUT_PULLUP:
    case GPIO_PM_INPUT_PULLDOWN:
    case GPIO_PM_INPUT_ANALOG:
    case GPIO_PM_OUTPUT_ANALOG:
        return -1;
    }
    return 0;
}

#endif

static void cc3200_init(void)
{
    PRCMCC3200MCUInit();
    ti_arm_mcu_systick_init();
}

static unsigned long cc3200_get_system_clock(void)
{
    return system_clock;
}

const platform_t platform = {
    .serial = {
        .enable = ti_arm_mcu_uart_enable,
        .read = buffered_serial_read,
        .write = ti_arm_mcu_serial_write,
        .irq_enable = ti_arm_mcu_serial_irq_enable,
    },
#ifdef CONFIG_GPIO
    .gpio = {
        .digital_write = ti_arm_mcu_gpio_digital_write,
        .digital_read = ti_arm_mcu_gpio_digital_read,
        .analog_read = ti_arm_mcu_gpio_analog_read,
        .set_pin_mode = cc3200_set_pin_mode,
        .set_port_val = ti_arm_mcu_gpio_set_port_val,
        .get_port_val = ti_arm_mcu_gpio_get_port_val,
    },
#endif
    .init = cc3200_init,
    .meminfo = cortex_m_meminfo,
    .panic = cortex_m_panic,
    .select = ti_arm_mcu_select,
    .get_time_from_boot = cortex_m_get_time_from_boot,
    .get_system_clock = cc3200_get_system_clock,
    .msleep = ti_arm_mcu_msleep,
};
