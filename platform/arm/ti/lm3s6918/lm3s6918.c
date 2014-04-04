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
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "driverlib/rom_map.h"
#include "util/debug.h"
#include "drivers/gpio/gpio_platform.h"
#include "drivers/serial/serial_platform.h"
#include "platform/platform.h"
#include "platform/arm/cortex-m.h"
#include "platform/arm/ti/ti_arm_mcu.h"
#include "platform/arm/ti/lm3s6918/lm3s6918.h"

const ti_arm_mcu_gpio_port_t ti_arm_mcu_gpio_ports[] = {
    [GPIO_PORT_A] = { SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, INT_GPIOA },
    [GPIO_PORT_B] = { SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, INT_GPIOB },
    [GPIO_PORT_C] = { SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, INT_GPIOC },
    [GPIO_PORT_D] = { SYSCTL_PERIPH_GPIOD, GPIO_PORTD_BASE, INT_GPIOD },
    [GPIO_PORT_E] = { SYSCTL_PERIPH_GPIOE, GPIO_PORTE_BASE, INT_GPIOE },
    [GPIO_PORT_F] = { SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, INT_GPIOF },
    [GPIO_PORT_G] = { SYSCTL_PERIPH_GPIOG, GPIO_PORTG_BASE, INT_GPIOG },
};

const ti_arm_mcu_uart_t ti_arm_mcu_uarts[] = {
    [UART0] = { SYSCTL_PERIPH_UART0, UART0_BASE, INT_UART0, PA0, PA1 },
    [UART1] = { SYSCTL_PERIPH_UART1, UART1_BASE, INT_UART1, PD2, PD3 },
};

const ti_arm_mcu_ssi_t ti_arm_mcu_ssis[] = {
#define SSI_DEF(num, clkpin, fsspin, rxpin, txpin) \
    [SSI##num] = { \
        .periph = SYSCTL_PERIPH_SSI##num, \
        .base = SSI##num##_BASE, \
        .clk = clkpin, \
        .fss = fsspin, \
        .rx = rxpin, \
        .tx = txpin, \
    }
    SSI_DEF(0, PA2, PA3, PA4, PA5),
    SSI_DEF(1, PE0, PE1, PE2, PE3)
};

const ti_arm_mcu_timer_t ti_arm_mcu_timers[] = {
};

const ti_arm_mcu_gpio_pin_t ti_arm_mcu_gpio_pins[] = {
};

#ifdef CONFIG_GPIO

static void lm3s6918_set_locked_pin_as_gpio(int pin)
{
    unsigned long base = ti_arm_mcu_gpio_base(pin);

    tp_warn(("Setting locked JTAG pin as GPIO\n"));

    HWREG(base + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(base + GPIO_O_CR) = GPIO_BIT(pin);
    HWREG(base + GPIO_O_AFSEL) &= ~GPIO_BIT(pin);
    HWREG(base + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(base + GPIO_O_CR) = 0x00;
    HWREG(base + GPIO_O_LOCK) = 0;
}

static int lm3s6918_set_pin_mode(int pin, gpio_pin_mode_t mode)
{
    ti_arm_mcu_periph_enable(ti_arm_mcu_gpio_periph(pin));

    if (pin == PB7 || pin == PC0 || pin == PC1 || pin == PC2 || pin == PC3)
        lm3s6918_set_locked_pin_as_gpio(pin);

    switch (mode)
    {
    case GPIO_PM_INPUT:
        ti_arm_mcu_gpio_input(pin);
        ti_arm_mcu_pin_config(pin, GPIO_PIN_TYPE_STD);
        break;
    case GPIO_PM_OUTPUT:
        ti_arm_mcu_pin_mode_output(pin);
        ti_arm_mcu_pin_config(pin, GPIO_PIN_TYPE_STD);
        break;
    case GPIO_PM_INPUT_PULLUP:
        ti_arm_mcu_gpio_input(pin);
        ti_arm_mcu_pin_config(pin, GPIO_PIN_TYPE_STD_WPU);
        break;
    case GPIO_PM_INPUT_PULLDOWN:
        ti_arm_mcu_gpio_input(pin);
        ti_arm_mcu_pin_config(pin, GPIO_PIN_TYPE_STD_WPD);
        break;
    default:
        return -1;
    }
    return 0;
}
#endif

static void lm3s6918_init(void)
{
    /* If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
     * a workaround to allow the PLL to operate reliably.
     */
    if (REVISION_IS_A2)
        SysCtlLDOSet(SYSCTL_LDO_2_75V);

    /* Set the clocking to run from the PLL */
    //
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
        SYSCTL_XTAL_8MHZ);

    /* Enable processor interrupts */
    IntMasterEnable();

    ti_arm_mcu_systick_init();
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
        .set_pin_mode = lm3s6918_set_pin_mode,
        .set_port_val = ti_arm_mcu_gpio_set_port_val,
        .get_port_val = ti_arm_mcu_gpio_get_port_val,
    },
#endif
#ifdef CONFIG_SPI
    .spi = {
        .init = ti_arm_mcu_spi_init,
        .reconf = ti_arm_mcu_spi_reconf,
        .set_max_speed = ti_arm_mcu_spi_set_max_speed,
        .send = ti_arm_mcu_spi_send,
        .receive = ti_arm_mcu_spi_receive,
    },
#endif
    .init = lm3s6918_init,
    .meminfo = cortex_m_meminfo,
    .panic = cortex_m_panic,
    .select = ti_arm_mcu_select,
    .get_ticks_from_boot = cortex_m_get_ticks_from_boot,
    .get_system_clock = ti_arm_mcu_get_system_clock,
    .msleep = ti_arm_mcu_msleep,
};
