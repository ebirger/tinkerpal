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
#include "driverlib/pwm.h"
#include "driverlib/adc.h"
#include "driverlib/rom_map.h"
#include "util/debug.h"
#include "drivers/gpio/gpio_platform.h"
#include "platform/platform.h"
#include "platform/ticks.h"
#include "platform/arm/cortex-m.h"
#include "platform/arm/ti/ti_arm_mcu.h"
#include "platform/arm/ti/tm4c1294/tm4c1294.h"

#define PLATFORM_CHIPSET_H "platform/arm/ti/tm4c1294/tm4c1294.chip"

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
#define TI_UART_DEF(num, rx, tx) \
    [UART##num] = { \
        .periph = SYSCTL_PERIPH_UART##num, \
        .base = UART##num##_BASE, \
        .irq = INT_UART##num, \
        .rxpin = rx, \
        .txpin = tx, \
        .rx_af = GPIO_##rx##_U##num##RX, \
        .tx_af = GPIO_##tx##_U##num##TX, \
    },

#include "platform/chipset.h"
};

const ti_arm_mcu_ssi_t ti_arm_mcu_ssis[] = {
#define TI_SSI_DEF(num, clkpin, fsspin, rxpin, txpin) \
    [SSI##num] = { \
        .periph = SYSCTL_PERIPH_SSI##num, \
        .base = SSI##num##_BASE, \
        .clk = clkpin, \
        .fss = fsspin, \
        .rx = rxpin, \
        .tx = txpin, \
        .clk_af = GPIO_##clkpin##_SSI##num##CLK, \
        .fss_af = GPIO_##fsspin##_SSI##num##FSS, \
        .rx_af = GPIO_##rxpin##_SSI##num##XDAT0, \
        .tx_af = GPIO_##txpin##_SSI##num##XDAT1, \
    },

#include "platform/chipset.h"
};

const ti_arm_mcu_i2c_t ti_arm_mcu_i2cs[] = {
#define TI_I2C_DEF(num, sclpin, sdapin) \
    [I2C##num] = { \
        .periph = SYSCTL_PERIPH_I2C##num, \
        .base = I2C##num##_BASE, \
        .scl = sclpin, \
        .sda = sdapin, \
        .scl_af = GPIO_##sclpin##_I2C##num##SCL, \
        .sda_af = GPIO_##sdapin##_I2C##num##SDA, \
    },

#include "platform/chipset.h"
};

const ti_arm_mcu_timer_t ti_arm_mcu_timers[] = {
};

const ti_arm_mcu_gpio_pin_t ti_arm_mcu_gpio_pins[] = {
};

const ti_arm_mcu_pwm_t ti_arm_mcu_pwms[] = {
#define TI_PWM_DEF(_pin, _base, _gen, _bit) \
    { \
        .periph = SYSCTL_PERIPH_PWM##_base, \
        .base = PWM##_base##_BASE, \
        .gen = PWM_GEN_##_gen, \
        .out = PWM_OUT_##_bit, \
        .out_bit = PWM_OUT_##_bit##_BIT, \
        .pin = _pin, \
        .af = GPIO_##_pin##_M##_base##PWM##_bit \
    },
#include "platform/chipset.h"
    {}
};

const ti_arm_mcu_usbd_params_t ti_arm_mcu_usbd_params = {
#define TI_USBD_DEF(dp, dm) \
    .dp_pin = dp, \
    .dm_pin = dm,

#include "platform/chipset.h"
};

static unsigned long system_clock;

#ifdef CONFIG_GPIO

static int tm4c1294_set_pin_mode(int pin, gpio_pin_mode_t mode)
{
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
        return ti_arm_mcu_pin_mode_pwm(pin);
    }
    return 0;
}

#endif

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
        .enable = ti_arm_mcu_uart_enable,
        .read = buffered_serial_read,
        .write = ti_arm_mcu_serial_write,
        .irq_enable = ti_arm_mcu_serial_irq_enable,
    },
    .mem = {
        .info = cortex_m_meminfo,
    },
#ifdef CONFIG_GPIO
    .gpio = {
        .digital_write = ti_arm_mcu_gpio_digital_write,
        .digital_read = ti_arm_mcu_gpio_digital_read,
        .analog_read = ti_arm_mcu_gpio_analog_read,
        .pwm_start = ti_arm_mcu_gpio_pwm_start,
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
#ifdef CONFIG_I2C
    .i2c = {
        .init = ti_arm_mcu_i2c_init,
        .reg_write = ti_arm_mcu_i2c_reg_write,
    },
#endif
#ifdef CONFIG_USB_DEVICE
    .usb = {
        .init = ti_arm_mcu_usb_init,
        .connect = ti_arm_mcu_usb_connect,
        .ep_cfg = ti_arm_mcu_usb_ep_cfg,
        .ep_data_ack = ti_arm_mcu_usb_ep_data_ack,
        .ep_data_get = ti_arm_mcu_usb_ep_data_get,
        .ep_data_send = ti_arm_mcu_usb_ep_data_send,
        .set_addr = ti_arm_mcu_usb_set_addr,
    },
#endif
    .init = tm4c1294_init,
    .panic = cortex_m_panic,
    .select = ti_arm_mcu_select,
    .get_time_from_boot = gen_get_time_from_boot,
    .get_system_clock = tm4c1294_get_system_clock,
    .msleep = ti_arm_mcu_msleep,
};
