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
#include "platform/arm/ti/lm4f120xl/lm4f120xl.h"

const ti_arm_mcu_gpio_port_t ti_arm_mcu_gpio_ports[] = {
    [GPIO_PORT_A] = { SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, INT_GPIOA },
    [GPIO_PORT_B] = { SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, INT_GPIOB },
    [GPIO_PORT_C] = { SYSCTL_PERIPH_GPIOC, GPIO_PORTC_BASE, INT_GPIOC },
    [GPIO_PORT_D] = { SYSCTL_PERIPH_GPIOD, GPIO_PORTD_BASE, INT_GPIOD },
    [GPIO_PORT_E] = { SYSCTL_PERIPH_GPIOE, GPIO_PORTE_BASE, INT_GPIOE },
    [GPIO_PORT_F] = { SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, INT_GPIOF }
};

const ti_arm_mcu_uart_t ti_arm_mcu_uarts[] = {
#define UART_DEF(num, rx, tx) \
    [UART##num] = { \
        .periph = SYSCTL_PERIPH_UART##num, \
        .base = UART##num##_BASE, \
        .irq = INT_UART##num, \
        .rxpin = rx, \
        .txpin = tx, \
        .rx_af = GPIO_##rx##_U##num##RX, \
        .tx_af = GPIO_##tx##_U##num##TX, \
    }
    UART_DEF(0, PA0, PA1),
    UART_DEF(1, PB0, PB1),
    UART_DEF(2, PD6, PD7),
    UART_DEF(3, PC6, PC7),
    UART_DEF(4, PC4, PC5),
    UART_DEF(5, PE4, PE5),
    UART_DEF(6, PD4, PD5),
    UART_DEF(7, PE0, PE1),
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
        .clk_af = GPIO_##clkpin##_SSI##num##CLK, \
        .fss_af = GPIO_##fsspin##_SSI##num##FSS, \
        .rx_af = GPIO_##rxpin##_SSI##num##RX, \
        .tx_af = GPIO_##txpin##_SSI##num##TX, \
    }
    SSI_DEF(0, PA2, PA3, PA4, PA5),
    SSI_DEF(1, PD0, PD1, PD2, PD3),
};

const ti_arm_mcu_timer_t ti_arm_mcu_timers[] = {
    [ TIMER0 ] = { SYSCTL_PERIPH_TIMER0, TIMER0_BASE },
    [ TIMER1 ] = { SYSCTL_PERIPH_TIMER1, TIMER1_BASE },
    [ TIMER2 ] = { SYSCTL_PERIPH_TIMER2, TIMER2_BASE },
    [ TIMER3 ] = { SYSCTL_PERIPH_TIMER3, TIMER3_BASE },
    [ TIMER4 ] = { SYSCTL_PERIPH_TIMER4, TIMER4_BASE },
    [ TIMER5 ] = { SYSCTL_PERIPH_TIMER5, TIMER5_BASE },
    [ WTIMER0 ] = { SYSCTL_PERIPH_WTIMER0, WTIMER0_BASE },
    [ WTIMER1 ] = { SYSCTL_PERIPH_WTIMER1, WTIMER1_BASE },
    [ WTIMER2 ] = { SYSCTL_PERIPH_WTIMER2, WTIMER2_BASE },
    [ WTIMER3 ] = { SYSCTL_PERIPH_WTIMER3, WTIMER3_BASE },
    [ WTIMER4 ] = { SYSCTL_PERIPH_WTIMER4, WTIMER4_BASE },
    [ WTIMER5 ] = { SYSCTL_PERIPH_WTIMER5, WTIMER5_BASE }
};

const ti_arm_mcu_i2c_t ti_arm_mcu_i2cs[] = {
#define I2C_DEF(num, sclpin, sdapin) \
    [I2C##num] = { \
        .periph = SYSCTL_PERIPH_I2C##num, \
        .base = I2C##num##_MASTER_BASE, \
        .scl = sclpin, \
        .sda = sdapin, \
        .scl_af = GPIO_##sclpin##_I2C##num##SCL, \
        .sda_af = GPIO_##sdapin##_I2C##num##SDA, \
    }
    I2C_DEF(0, PB2, PB3),
    I2C_DEF(1, PA6, PA7),
    I2C_DEF(2, PE4, PE5),
    I2C_DEF(3, PD0, PD1),
};

/* Notes:
 * - PC4/5 can be used for either UART1 or UART4. Currently going for 4, 
 * since PB0/1 can be used for UART1
 * - PF timers are shared with PB
 */
const ti_arm_mcu_gpio_pin_t ti_arm_mcu_gpio_pins[] = {
    [ PA0 ] = {-1, 0, -1},
    [ PA1 ] = {-1, 0, -1},
    [ PA2 ] = {-1, 0, -1},
    [ PA3 ] = {-1, 0, -1},
    [ PA4 ] = {-1, 0, -1},
    [ PA5 ] = {-1, 0, -1},
    [ PA6 ] = {-1, 0, -1},
    [ PA7 ] = {-1, 0, -1},
    [ PB0 ] = {TIMER2, GPIO_PB0_T2CCP0, -1},
    [ PB1 ] = {TIMER2, GPIO_PB1_T2CCP1, -1},
    [ PB2 ] = {TIMER3, GPIO_PB2_T3CCP0, -1},
    [ PB3 ] = {TIMER3, GPIO_PB3_T3CCP1, -1},
    [ PB4 ] = {TIMER1, GPIO_PB4_T1CCP0, ADC_CTL_CH10},
    [ PB5 ] = {TIMER1, GPIO_PB5_T1CCP1, ADC_CTL_CH11},
    [ PB6 ] = {TIMER0, GPIO_PB6_T0CCP0, -1},
    [ PB7 ] = {TIMER0, GPIO_PB7_T0CCP1, -1},
    [ PC0 ] = {TIMER4, GPIO_PC0_T4CCP0, -1},
    [ PC1 ] = {TIMER4, GPIO_PC1_T4CCP1, -1},
    [ PC2 ] = {TIMER5, GPIO_PC2_T5CCP0, -1},
    [ PC3 ] = {TIMER5, GPIO_PC3_T5CCP1, -1},
    [ PC4 ] = {WTIMER0, GPIO_PC4_WT0CCP0, -1},
    [ PC5 ] = {WTIMER0, GPIO_PC5_WT0CCP1, -1},
    [ PC6 ] = {WTIMER1, GPIO_PC6_WT1CCP0, -1},
    [ PC7 ] = {WTIMER1, GPIO_PC7_WT1CCP1, -1},
    [ PD0 ] = {WTIMER2, GPIO_PD0_WT2CCP0, ADC_CTL_CH7},
    [ PD1 ] = {WTIMER2, GPIO_PD1_WT2CCP1, ADC_CTL_CH6},
    [ PD2 ] = {WTIMER3, GPIO_PD2_WT3CCP0, ADC_CTL_CH5},
    [ PD3 ] = {WTIMER3, GPIO_PD3_WT3CCP1, ADC_CTL_CH4},
    [ PD4 ] = {WTIMER4, GPIO_PD4_WT4CCP0, -1},
    [ PD5 ] = {WTIMER4, GPIO_PD5_WT4CCP1, -1},
    [ PD6 ] = {WTIMER5, GPIO_PD6_WT5CCP0, -1},
    [ PD7 ] = {WTIMER5, GPIO_PD7_WT5CCP1, -1},
    [ PE0 ] = {-1, 0, ADC_CTL_CH3},
    [ PE1 ] = {-1, 0, ADC_CTL_CH2},
    [ PE2 ] = {-1, 0, ADC_CTL_CH1},
    [ PE3 ] = {-1, 0, ADC_CTL_CH0},
    [ PE4 ] = {-1, 0, ADC_CTL_CH9},
    [ PE5 ] = {-1, 0, ADC_CTL_CH8},
    [ PE6 ] = {-1, 0, -1},
    [ PE7 ] = {-1, 0, -1},
    [ PF0 ] = {TIMER0, GPIO_PF0_T0CCP0, -1},
    [ PF1 ] = {TIMER0, GPIO_PF1_T0CCP1, -1},
    [ PF2 ] = {TIMER1, GPIO_PF2_T1CCP0, -1},
    [ PF3 ] = {TIMER1, GPIO_PF3_T1CCP1, -1},
    [ PF4 ] = {TIMER2, GPIO_PF4_T2CCP0, -1},
    [ PF5 ] = {-1, 0, -1},
    [ PF6 ] = {-1, 0, -1},
    [ PF7 ] = {-1, 0, -1}
};

#define HALF_TIMER(p) ((p) & 0x1 ? TIMER_B : TIMER_A) /* even pins use TIMER_A, odd pins use TIMER_B */
#define TIMER(p) (&ti_arm_mcu_timers[ti_arm_mcu_gpio_pins[(p)].timer])
#define TIMER_SET(p, t) ROM_TimerMatchSet(TIMER(p)->base, HALF_TIMER(p), t)

#ifdef CONFIG_GPIO

static void pinmode_pwm(int pin)
{
    const ti_arm_mcu_timer_t *timer;
    unsigned long half_timer;

    timer = TIMER(pin);
    half_timer = HALF_TIMER(pin);

    ti_arm_mcu_periph_enable(timer->periph);

    /* Configure GPIO */
    ti_arm_mcu_pin_mode_timer(pin);

    /* Configure Timer */
    ROM_TimerConfigure(timer->base, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | 
        TIMER_CFG_B_PWM));
    ROM_TimerPrescaleSet(timer->base, half_timer, 0); // ~1230 Hz PWM
    /* Timer will load this value on timeout */
    ROM_TimerLoadSet(timer->base, half_timer, 65279);
    /* Initial duty cycle of 0 */
    TIMER_SET(pin, 65278);
    /* PWM should not be inverted */
    ROM_TimerControlLevel(timer->base, half_timer, 0);
    
    ROM_TimerEnable(timer->base, half_timer);
}

static int lm4120xl_set_pin_mode(int pin, gpio_pin_mode_t mode)
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
        pinmode_pwm(pin);
        break;
    }
    return 0;
}

static void ti_arm_mcu_gpio_pwm_start(int pin, int freq, int duty_cycle) 
{
    int int_val;

    /* Ignoring freq for now */
    int_val = 255 * duty_cycle / 100;

    if (int_val > 255)
        int_val = 255;
    
    if (int_val == 0)
        int_val = 65278;
    else
        int_val = 65280 - (256 * int_val);

    TIMER_SET(pin, int_val);
}

#endif

static void buttons_init(void)
{
#ifdef CONFIG_GPIO
    /* Enable the GPIO port that is used for the on-board Buttons*/
    ti_arm_mcu_periph_enable(SYSCTL_PERIPH_GPIOF);

    /* Unlock PF0 so we can change it to a GPIO input
     * Once we have enabled (unlocked) the commit register then re-lock it
     * to prevent further changes.  PF0 is muxed with NMI thus a special case.
     */
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
    lm4120xl_set_pin_mode(PF0, GPIO_PM_INPUT_PULLUP);
    lm4120xl_set_pin_mode(PF4, GPIO_PM_INPUT_PULLUP);
#endif
}

static void lm4f120xl_init(void)
{
    /* Enable lazy stacking for interrupt handlers.  This allows floating-point
     * instructions to be used within interrupt handlers, but at the expense of
     * extra stack usage.
     */
    ROM_FPULazyStackingEnable();

    /* Set the clocking to run directly from the crystal at 80 MHz */
    ROM_SysCtlClockSet(SYSCTL_RCC2_DIV400 | SYSCTL_SYSDIV_2_5 |
        SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
                       
    ti_arm_mcu_systick_init();
    buttons_init(); 
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
        .pwm_start = ti_arm_mcu_gpio_pwm_start,
        .analog_read = ti_arm_mcu_gpio_analog_read,
        .set_pin_mode = lm4120xl_set_pin_mode,
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
    .init = lm4f120xl_init,
    .meminfo = cortex_m_meminfo,
    .panic = cortex_m_panic,
    .select = ti_arm_mcu_select,
    .get_time_from_boot = ti_arm_mcu_get_time_from_boot,
    .get_system_clock = ti_arm_mcu_get_system_clock,
    .msleep = ti_arm_mcu_msleep,
};
