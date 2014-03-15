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
#include "platform/arm/stellaris/stellaris.h"
#include "platform/arm/stellaris/lm3s6965/lm3s6965.h"

const stellaris_gpio_port_t stellaris_gpio_ports[] = {
    [GPIO_PORT_A] = { SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, INT_GPIOA },
    [GPIO_PORT_B] = { SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, INT_GPIOB },
    [GPIO_PORT_C] = { SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, INT_GPIOC },
    [GPIO_PORT_D] = { SYSCTL_PERIPH_GPIOD, GPIO_PORTD_BASE, INT_GPIOD },
    [GPIO_PORT_E] = { SYSCTL_PERIPH_GPIOE, GPIO_PORTE_BASE, INT_GPIOE },
    [GPIO_PORT_F] = { SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, INT_GPIOF },
};

const stellaris_uart_t stellaris_uarts[] = {
    [UART0] = { SYSCTL_PERIPH_UART0, UART0_BASE, INT_UART0, PA0, PA1 },
};

const stellaris_ssi_t stellaris_ssis[] = {
    [SSI0] = { SYSCTL_PERIPH_SSI0, SSI0_BASE, PA2, PA3, PA4, PA5 },
};

const stellaris_timer_t stellaris_timers[] = {
};

const stellaris_gpio_pin_t stellaris_gpio_pins[] = {
    [ PA0 ] = {-1, -1, -1, -1, -1},
    [ PA1 ] = {-1, -1, -1, -1, -1},
    [ PA2 ] = {-1, -1, -1, -1, -1},
    [ PA3 ] = {-1, -1, -1, -1, -1},
    [ PA4 ] = {-1, -1, -1, -1, -1},
    [ PA5 ] = {-1, -1, -1, -1, -1},
    [ PA6 ] = {-1, -1, -1, -1, -1},
    [ PA7 ] = {-1, -1, -1, -1, -1},
    [ PB0 ] = {-1, -1, -1, -1, -1},
    [ PB1 ] = {-1, -1, -1, -1, -1},
    [ PB2 ] = {-1, -1, -1, -1, -1},
    [ PB3 ] = {-1, -1, -1, -1, -1},
    [ PB4 ] = {-1, -1, -1, -1, -1},
    [ PB5 ] = {-1, -1, -1, -1, -1},
    [ PB6 ] = {-1, -1, -1, -1, -1},
    [ PB7 ] = {-1, -1, -1, -1, -1},
    [ PC0 ] = {-1, -1, -1, -1, -1},
    [ PC1 ] = {-1, -1, -1, -1, -1},
    [ PC2 ] = {-1, -1, -1, -1, -1},
    [ PC3 ] = {-1, -1, -1, -1, -1},
    [ PC4 ] = {-1, -1, -1, -1, -1},
    [ PC5 ] = {-1, -1, -1, -1, -1},
    [ PC6 ] = {-1, -1, -1, -1, -1},
    [ PC7 ] = {-1, -1, -1, -1, -1},
    [ PD0 ] = {-1, -1, -1, -1, -1},
    [ PD1 ] = {-1, -1, -1, -1, -1},
    [ PD2 ] = {-1, -1, -1, -1, -1},
    [ PD3 ] = {-1, -1, -1, -1, -1},
    [ PD4 ] = {-1, -1, -1, -1, -1},
    [ PD5 ] = {-1, -1, -1, -1, -1},
    [ PD6 ] = {-1, -1, -1, -1, -1},
    [ PD7 ] = {-1, -1, -1, -1, -1},
    [ PE0 ] = {-1, -1, -1, -1, -1},
    [ PE1 ] = {-1, -1, -1, -1, -1},
    [ PE2 ] = {-1, -1, -1, -1, -1},
    [ PE3 ] = {-1, -1, -1, -1, -1},
    [ PE4 ] = {-1, -1, -1, -1, -1},
    [ PE5 ] = {-1, -1, -1, -1, -1},
    [ PE6 ] = {-1, -1, -1, -1, -1},
    [ PE7 ] = {-1, -1, -1, -1, -1},
    [ PF0 ] = {-1, -1, -1, -1, -1},
    [ PF1 ] = {-1, -1, -1, -1, -1},
    [ PF2 ] = {-1, -1, -1, -1, -1},
    [ PF3 ] = {-1, -1, -1, -1, -1},
    [ PF4 ] = {-1, -1, -1, -1, -1},
    [ PF5 ] = {-1, -1, -1, -1, -1},
    [ PF6 ] = {-1, -1, -1, -1, -1},
    [ PF7 ] = {-1, -1, -1, -1, -1},
};

#ifdef CONFIG_GPIO

static void stellaris_gpio_analog_write(int pin, double value) 
{
    unsigned long period, width;

    /* Compute the PWM period based on the system clock */
    period = SysCtlClockGet() / 440;

    /* Set the PWM period  */
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN | 
        PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, period);

    width = (unsigned long)(value * period);
    /* Taking up too much of the period will result in having nothing */
    if (width > period - 5)
	width = period - 5;

    /* Set PWM0 duty cycle */
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, width);

    /* Enable the PWM0 output signals */
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);

    /* Enable the PWM generator */
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
}

static void pinmode_pwm(int pin)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    GPIOPinTypePWM(stellaris_gpio_base(pin), GPIO_BIT(pin));
}

static int lm3s6965_set_pin_mode(int pin, gpio_pin_mode_t mode)
{
    stellaris_periph_enable(stellaris_gpio_periph(pin));

    switch (mode)
    {
    case GPIO_PM_INPUT:
	stellaris_gpio_input(pin);
	stellaris_pin_config(pin, GPIO_PIN_TYPE_STD);
	break;
    case GPIO_PM_OUTPUT:
	stellaris_pin_mode_output(pin);
	stellaris_pin_config(pin, GPIO_PIN_TYPE_STD);
	break;
    case GPIO_PM_INPUT_PULLUP:
	stellaris_gpio_input(pin);
	stellaris_pin_config(pin, GPIO_PIN_TYPE_STD_WPU);
	break;
    case GPIO_PM_INPUT_PULLDOWN:
	stellaris_gpio_input(pin);
	stellaris_pin_config(pin, GPIO_PIN_TYPE_STD_WPD);
	break;
    case GPIO_PM_OUTPUT_ANALOG:
	pinmode_pwm(pin);
	break;
    default:
	return -1;
    }
    return 0;
}
#endif

static int stellaris_serial_enable(int u, int enabled)
{
    stellaris_uart_enable(u, enabled);
    return 0;
}

static void lm3s6965_init(void)
{
    /* Set the clocking to run directly from the crystal */
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
        SYSCTL_XTAL_8MHZ);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

    /* Enable processor interrupts */
    IntMasterEnable();

    stellaris_systick_init();
}

const platform_t platform = {
    .serial = {
	.enable = stellaris_serial_enable,
	.read = buffered_serial_read,
	.write = stellaris_serial_write,
	.irq_enable = stellaris_serial_irq_enable,
    },
#ifdef CONFIG_GPIO
    .gpio = {
	.digital_write = stellaris_gpio_digital_write,
	.digital_read = stellaris_gpio_digital_read,
	.analog_write = stellaris_gpio_analog_write,
	.set_pin_mode = lm3s6965_set_pin_mode,
	.set_port_val = stellaris_gpio_set_port_val,
	.get_port_val = stellaris_gpio_get_port_val,
    },
#endif
#ifdef CONFIG_SPI
    .spi = {
	.init = stellaris_spi_init,
	.reconf = stellaris_spi_reconf,
	.set_max_speed = stellaris_spi_set_max_speed,
	.send = stellaris_spi_send,
	.receive = stellaris_spi_receive,
    },
#endif
    .init = lm3s6965_init,
    .meminfo = cortex_m_meminfo,
    .panic = cortex_m_panic,
    .select = stellaris_select,
    .get_ticks_from_boot = cortex_m_get_ticks_from_boot,
    .get_system_clock = stellaris_get_system_clock,
    .msleep = stellaris_msleep,
};
