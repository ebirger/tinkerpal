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
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
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
#include "platform/arm/ti/tm4c1294/tm4c1294.h"

const ti_arm_mcu_gpio_port_t ti_arm_mcu_gpio_ports[] = {
    [GPIO_PORT_A] = { SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, INT_GPIOA },
    [GPIO_PORT_B] = { SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, INT_GPIOB },
    [GPIO_PORT_C] = { SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, INT_GPIOC },
    [GPIO_PORT_D] = { SYSCTL_PERIPH_GPIOD, GPIO_PORTD_BASE, INT_GPIOD },
    [GPIO_PORT_E] = { SYSCTL_PERIPH_GPIOE, GPIO_PORTE_BASE, INT_GPIOE },
    [GPIO_PORT_F] = { SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, INT_GPIOF },
    [GPIO_PORT_G] = { SYSCTL_PERIPH_GPIOG, GPIO_PORTG_BASE, INT_GPIOG },
    [GPIO_PORT_H] = { SYSCTL_PERIPH_GPIOH, GPIO_PORTH_BASE, INT_GPIOH },
    [GPIO_PORT_J] = { SYSCTL_PERIPH_GPIOJ, GPIO_PORTJ_BASE, INT_GPIOJ },
    [GPIO_PORT_K] = { SYSCTL_PERIPH_GPIOK, GPIO_PORTK_BASE, INT_GPIOK },
    [GPIO_PORT_L] = { SYSCTL_PERIPH_GPIOL, GPIO_PORTL_BASE, INT_GPIOL },
    [GPIO_PORT_M] = { SYSCTL_PERIPH_GPIOM, GPIO_PORTM_BASE, INT_GPIOM },
    [GPIO_PORT_N] = { SYSCTL_PERIPH_GPION, GPIO_PORTN_BASE, INT_GPION },
    [GPIO_PORT_P] = { SYSCTL_PERIPH_GPIOP, GPIO_PORTP_BASE, -1 },
    [GPIO_PORT_Q] = { SYSCTL_PERIPH_GPIOQ, GPIO_PORTQ_BASE, -1 }
};

const ti_arm_mcu_uart_t ti_arm_mcu_uarts[] = {
    [UART0] = { SYSCTL_PERIPH_UART0, UART0_BASE, INT_UART0, PA0, PA1 },
    [UART1] = { SYSCTL_PERIPH_UART1, UART1_BASE, INT_UART1, PB0, PB1 },
    [UART2] = { SYSCTL_PERIPH_UART2, UART2_BASE, INT_UART2, PA6, PA7 },
    [UART3] = { SYSCTL_PERIPH_UART3, UART3_BASE, INT_UART3, PA4, PA5 },
    [UART4] = { SYSCTL_PERIPH_UART4, UART4_BASE, INT_UART4, PA2, PA3 },
    [UART5] = { SYSCTL_PERIPH_UART5, UART5_BASE, INT_UART5, PC6, PC7 },
    [UART6] = { SYSCTL_PERIPH_UART6, UART6_BASE, INT_UART6, PP0, PP1 },
    [UART7] = { SYSCTL_PERIPH_UART7, UART7_BASE, INT_UART7, PC4, PC5 },
};

const ti_arm_mcu_ssi_t ti_arm_mcu_ssis[] = {
    [SSI0] = {
	.periph = SYSCTL_PERIPH_SSI0,
	.base = SSI0_BASE,
	.clk = PA2,
	.fss = PA3,
	.rx = PA4,
	.tx = PA5, 
    },
    [SSI1] = { 
	.periph = SYSCTL_PERIPH_SSI1,
       	.base = SSI1_BASE,
       	.clk = PB5,
       	.fss = PB4,
       	.rx = PE4,
       	.tx = PE5,
    },
    [SSI2] = { 
	.periph = SYSCTL_PERIPH_SSI2,
       	.base = SSI2_BASE,
       	.clk = PD3,
       	.fss = PD2,
       	.rx = PD1,
       	.tx = PD0,
    },
};

const ti_arm_mcu_timer_t ti_arm_mcu_timers[] = {
};

const ti_arm_mcu_gpio_pin_t ti_arm_mcu_gpio_pins[] = {
    [ PA0 ] = {-1, -1, -1, GPIO_PA0_U0RX, -1},
    [ PA1 ] = {-1, -1, -1, GPIO_PA1_U0TX, -1},
    [ PA2 ] = {-1, -1, -1, GPIO_PA2_U4RX, -1},
    [ PA3 ] = {-1, -1, -1, GPIO_PA3_U4TX, -1},
    [ PA4 ] = {-1, -1, -1, GPIO_PA4_U3RX, -1},
    [ PA5 ] = {-1, -1, -1, GPIO_PA5_U3TX, -1},
    [ PA6 ] = {-1, -1, -1, GPIO_PA6_U2RX, -1},
    [ PA7 ] = {-1, -1, -1, GPIO_PA7_U2TX, -1},
    [ PB0 ] = {-1, -1, -1, GPIO_PB0_U1RX, -1},
    [ PB1 ] = {-1, -1, -1, GPIO_PB1_U1TX, -1},
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
    [ PC4 ] = {-1, -1, -1, GPIO_PC4_U7RX, -1},
    [ PC5 ] = {-1, -1, -1, GPIO_PC5_U7TX, -1},
    [ PC6 ] = {-1, -1, -1, GPIO_PC6_U5RX, -1},
    [ PC7 ] = {-1, -1, -1, GPIO_PC7_U5TX, -1},
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
    [ PG0 ] = {-1, -1, -1, -1, -1},
    [ PG1 ] = {-1, -1, -1, -1, -1},
    [ PG2 ] = {-1, -1, -1, -1, -1},
    [ PG3 ] = {-1, -1, -1, -1, -1},
    [ PG4 ] = {-1, -1, -1, -1, -1},
    [ PG5 ] = {-1, -1, -1, -1, -1},
    [ PG6 ] = {-1, -1, -1, -1, -1},
    [ PG7 ] = {-1, -1, -1, -1, -1},
    [ PH0 ] = {-1, -1, -1, -1, -1},
    [ PH1 ] = {-1, -1, -1, -1, -1},
    [ PH2 ] = {-1, -1, -1, -1, -1},
    [ PH3 ] = {-1, -1, -1, -1, -1},
    [ PH4 ] = {-1, -1, -1, -1, -1},
    [ PH5 ] = {-1, -1, -1, -1, -1},
    [ PH6 ] = {-1, -1, -1, -1, -1},
    [ PH7 ] = {-1, -1, -1, -1, -1},
    [ PJ0 ] = {-1, -1, -1, -1, -1},
    [ PJ1 ] = {-1, -1, -1, -1, -1},
    [ PJ2 ] = {-1, -1, -1, -1, -1},
    [ PJ3 ] = {-1, -1, -1, -1, -1},
    [ PJ4 ] = {-1, -1, -1, -1, -1},
    [ PJ5 ] = {-1, -1, -1, -1, -1},
    [ PJ6 ] = {-1, -1, -1, -1, -1},
    [ PJ7 ] = {-1, -1, -1, -1, -1},
    [ PK0 ] = {-1, -1, -1, -1, -1},
    [ PK1 ] = {-1, -1, -1, -1, -1},
    [ PK2 ] = {-1, -1, -1, -1, -1},
    [ PK3 ] = {-1, -1, -1, -1, -1},
    [ PK4 ] = {-1, -1, -1, -1, -1},
    [ PK5 ] = {-1, -1, -1, -1, -1},
    [ PK6 ] = {-1, -1, -1, -1, -1},
    [ PK7 ] = {-1, -1, -1, -1, -1},
    [ PL0 ] = {-1, -1, -1, -1, -1},
    [ PL1 ] = {-1, -1, -1, -1, -1},
    [ PL2 ] = {-1, -1, -1, -1, -1},
    [ PL3 ] = {-1, -1, -1, -1, -1},
    [ PL4 ] = {-1, -1, -1, -1, -1},
    [ PL5 ] = {-1, -1, -1, -1, -1},
    [ PL6 ] = {-1, -1, -1, -1, -1},
    [ PL7 ] = {-1, -1, -1, -1, -1},
    [ PM0 ] = {-1, -1, -1, -1, -1},
    [ PM1 ] = {-1, -1, -1, -1, -1},
    [ PM2 ] = {-1, -1, -1, -1, -1},
    [ PM3 ] = {-1, -1, -1, -1, -1},
    [ PM4 ] = {-1, -1, -1, -1, -1},
    [ PM5 ] = {-1, -1, -1, -1, -1},
    [ PM6 ] = {-1, -1, -1, -1, -1},
    [ PM7 ] = {-1, -1, -1, -1, -1},
    [ PN0 ] = {-1, -1, -1, -1, -1},
    [ PN1 ] = {-1, -1, -1, -1, -1},
    [ PN2 ] = {-1, -1, -1, -1, -1},
    [ PN3 ] = {-1, -1, -1, -1, -1},
    [ PN4 ] = {-1, -1, -1, -1, -1},
    [ PN5 ] = {-1, -1, -1, -1, -1},
    [ PN6 ] = {-1, -1, -1, -1, -1},
    [ PN7 ] = {-1, -1, -1, -1, -1},
    [ PP0 ] = {-1, -1, -1, GPIO_PP0_U6RX, -1},
    [ PP1 ] = {-1, -1, -1, GPIO_PP1_U6TX, -1},
    [ PP2 ] = {-1, -1, -1, -1, -1},
    [ PP3 ] = {-1, -1, -1, -1, -1},
    [ PP4 ] = {-1, -1, -1, -1, -1},
    [ PP5 ] = {-1, -1, -1, -1, -1},
    [ PP6 ] = {-1, -1, -1, -1, -1},
    [ PP7 ] = {-1, -1, -1, -1, -1},
    [ PQ0 ] = {-1, -1, -1, -1, -1},
    [ PQ1 ] = {-1, -1, -1, -1, -1},
    [ PQ2 ] = {-1, -1, -1, -1, -1},
    [ PQ3 ] = {-1, -1, -1, -1, -1},
    [ PQ4 ] = {-1, -1, -1, -1, -1},
    [ PQ5 ] = {-1, -1, -1, -1, -1},
    [ PQ6 ] = {-1, -1, -1, -1, -1},
    [ PQ7 ] = {-1, -1, -1, -1, -1},
};

static unsigned long system_clock;

#ifdef CONFIG_GPIO

static int tm4c1294_set_pin_mode(int pin, gpio_pin_mode_t mode)
{
    /* Anti-brick JTAG Protection */
    if (pin >= PC0 && pin <= PC3) 
	return -1;

    if (mode == GPIO_PM_OUTPUT_ANALOG && ti_arm_mcu_gpio_pins[pin].timer == -1)
	return -1;

    ti_arm_mcu_periph_enable(ti_arm_mcu_gpio_periph(pin));

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
    case GPIO_PM_INPUT_ANALOG:
	if (ti_arm_mcu_pin_mode_adc(pin))
	    return -1;
	break;
    case GPIO_PM_OUTPUT_ANALOG:
	return -1;
    }
    return 0;
}

#endif

static int ti_arm_mcu_serial_enable(int u, int enabled)
{
    ti_arm_mcu_uart_enable(u, enabled);
    return 0;
}

static void tm4c1294_init(void)
{
    /* Set the clocking to run directly from the crystal at 120 MHz */
    system_clock = MAP_SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
	SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480, 120000000);
    
    ti_arm_mcu_systick_init();
}

static unsigned long tm4c1294_get_system_clock(void)
{
    return system_clock;
}

const platform_t platform = {
    .serial = {
	.enable = ti_arm_mcu_serial_enable,
	.read = buffered_serial_read,
	.write = ti_arm_mcu_serial_write,
	.irq_enable = ti_arm_mcu_serial_irq_enable,
    },
#ifdef CONFIG_GPIO
    .gpio = {
	.digital_write = ti_arm_mcu_gpio_digital_write,
	.digital_read = ti_arm_mcu_gpio_digital_read,
	.analog_read = ti_arm_mcu_gpio_analog_read,
	.set_pin_mode = tm4c1294_set_pin_mode,
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
    .init = tm4c1294_init,
    .meminfo = cortex_m_meminfo,
    .panic = cortex_m_panic,
    .select = ti_arm_mcu_select,
    .get_ticks_from_boot = cortex_m_get_ticks_from_boot,
    .get_system_clock = tm4c1294_get_system_clock,
    .msleep = ti_arm_mcu_msleep,
};
