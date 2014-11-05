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
#ifndef __TI_ARM_MCU_H__
#define __TI_ARM_MCU_H__

#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
#include "driverlib/sysctl.h"
#endif

typedef struct {
    unsigned long periph;
    unsigned long base;
    unsigned long irq;
    int rxpin;
    int txpin;
    int rx_af;
    int tx_af;
} ti_arm_mcu_uart_t;

typedef struct {
    unsigned long periph;
    unsigned long base;
    int irq;
} ti_arm_mcu_gpio_port_t;

typedef struct {
    unsigned long periph;
    unsigned long base;
    int clk;
    int fss;
    int rx;
    int tx;
    int clk_af;
    int fss_af;
    int rx_af;
    int tx_af;
} ti_arm_mcu_ssi_t;

typedef struct {
    unsigned long periph;
    unsigned long base;
} ti_arm_mcu_timer_t;

typedef struct {
    unsigned long periph;
    unsigned long base;
    unsigned long gen;
    unsigned long out;
    unsigned long out_bit;
    int pin;
    unsigned long af;
} ti_arm_mcu_pwm_t;

typedef struct {
    int timer;
    int timer_function;
    int adc_channel;
} ti_arm_mcu_gpio_pin_t;

typedef struct {
    unsigned long periph;
    unsigned long base;
    int scl;
    int sda;
    int scl_af;
    int sda_af;
} ti_arm_mcu_i2c_t;

typedef struct {
    int dp_pin;
    int dm_pin;
} ti_arm_mcu_usbd_params_t;

/* Defined in each specific target board */
extern const ti_arm_mcu_uart_t ti_arm_mcu_uarts[];
extern const ti_arm_mcu_gpio_port_t ti_arm_mcu_gpio_ports[];
extern const ti_arm_mcu_ssi_t ti_arm_mcu_ssis[];
extern const ti_arm_mcu_timer_t ti_arm_mcu_timers[];
extern const ti_arm_mcu_gpio_pin_t ti_arm_mcu_gpio_pins[];
extern const ti_arm_mcu_pwm_t ti_arm_mcu_pwms[];
extern const ti_arm_mcu_i2c_t ti_arm_mcu_i2cs[];
extern const ti_arm_mcu_usbd_params_t ti_arm_mcu_usbd_params;

void ti_arm_mcu_systick_init(void);
void ti_arm_mcu_get_time_from_boot(uint32_t *sec, uint32_t *usec);
unsigned long ti_arm_mcu_get_system_clock(void);
void ti_arm_mcu_msleep(double ms);

int ti_arm_mcu_uart_enable(int u, int enabled);
int ti_arm_mcu_uart_set_params(int u, const serial_params_t *params);
int ti_arm_mcu_select(int ms);
void ti_arm_mcu_serial_irq_enable(int u, int enable);
int ti_arm_mcu_serial_write(int u, char *buf, int size);

static inline void ti_arm_mcu_periph_enable(unsigned long periph)
{
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
    MAP_SysCtlPeripheralEnable(periph);
    MAP_SysCtlPeripheralSleepEnable(periph);
#elif defined(CONFIG_CC3200)
    MAP_PRCMPeripheralClkEnable(periph, PRCM_RUN_MODE_CLK);
#endif
}

static inline void ti_arm_mcu_periph_disable(unsigned long periph)
{
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
    MAP_SysCtlPeripheralDisable(periph);
#elif defined(CONFIG_CC3200)
    MAP_PRCMPeripheralClkDisable(periph, PRCM_RUN_MODE_CLK);
#endif
}

static inline void ti_arm_mcu_sleep(void)
{
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
    MAP_SysCtlSleep();
#elif defined(CONFIG_CC3200)
    /* Not implemented yet */
#endif
}

static inline unsigned long ti_arm_mcu_gpio_periph(int pin)
{
    return ti_arm_mcu_gpio_ports[GPIO_PORT(pin)].periph;
}

static inline unsigned long ti_arm_mcu_gpio_port_base(int port)
{
    return ti_arm_mcu_gpio_ports[port].base;
}

static inline unsigned long ti_arm_mcu_gpio_base(int pin)
{
    return ti_arm_mcu_gpio_port_base(GPIO_PORT(pin));
}

static inline void ti_arm_mcu_pin_mode_output(int pin)
{
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
    MAP_GPIOPinTypeGPIOOutput(ti_arm_mcu_gpio_base(pin), GPIO_BIT(pin));
#elif defined(CONFIG_CC3200)
    MAP_PinTypeGPIO(pin, PIN_MODE_0, false);
    MAP_GPIODirModeSet(ti_arm_mcu_gpio_base(pin), GPIO_BIT(pin),
        GPIO_DIR_MODE_OUT);
#endif
}

static inline void ti_arm_mcu_pin_config(int pin, int mode)
{
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
    MAP_GPIOPadConfigSet(ti_arm_mcu_gpio_base(pin), GPIO_BIT(pin), 
        GPIO_STRENGTH_8MA, mode);
#elif defined(CONFIG_CC3200)
    MAP_PinConfigSet(pin, PIN_STRENGTH_6MA, mode);
#endif
}

static inline int ti_arm_mcu_pin_mode_adc(int pin)
{
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
    if (ti_arm_mcu_gpio_pins[pin].adc_channel == -1)
        return -1;

    ti_arm_mcu_periph_enable(SYSCTL_PERIPH_ADC0);
    MAP_GPIOPinTypeADC(ti_arm_mcu_gpio_base(pin), GPIO_BIT(pin));
    return 0;
#elif defined(CONFIG_CC3200)
    tp_crit(("%s not implemented yet\n", __FUNCTION__));
    return -1;
#endif
}

static inline void ti_arm_mcu_pin_mode_timer(int pin)
{
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
    int timer_function;

    MAP_GPIODirModeSet(ti_arm_mcu_gpio_base(pin), GPIO_BIT(pin), 
        GPIO_DIR_MODE_HW);
    ti_arm_mcu_pin_config(pin, GPIO_PIN_TYPE_STD);
    MAP_GPIOPinTypeTimer(ti_arm_mcu_gpio_base(pin), GPIO_BIT(pin));
    if ((timer_function = ti_arm_mcu_gpio_pins[pin].timer_function) != -1)
        MAP_GPIOPinConfigure(timer_function);
#elif defined(CONFIG_CC3200)
    tp_crit(("%s not implemented yet\n", __FUNCTION__));
#endif
}

#ifdef CONFIG_PLAT_HAS_PWM
static inline const ti_arm_mcu_pwm_t *ti_arm_mcu_pin_pwm(int pin)
{
    const ti_arm_mcu_pwm_t *pwm;

    for (pwm = ti_arm_mcu_pwms; pwm->base && pwm->pin != pin; pwm++);
    return pwm;
}

int ti_arm_mcu_pin_mode_pwm(int pin);
void ti_arm_mcu_gpio_pwm_start(int pin, int freq, int duty_cycle);
#endif

#ifdef CONFIG_GPIO
void ti_arm_mcu_gpio_input(int pin);
void ti_arm_mcu_gpio_digital_write(int pin, int value);
int ti_arm_mcu_gpio_digital_read(int pin);
double ti_arm_mcu_gpio_analog_read(int pin);
void ti_arm_mcu_gpio_set_port_val(int port, unsigned short mask,
    unsigned short value);
unsigned short ti_arm_mcu_gpio_get_port_val(int port, unsigned short mask);
#endif
#ifdef CONFIG_SPI
int ti_arm_mcu_spi_init(int port);
void ti_arm_mcu_spi_reconf(int port);
void ti_arm_mcu_spi_set_max_speed(int port, unsigned long speed);
void ti_arm_mcu_spi_send(int port, unsigned long data);
unsigned long ti_arm_mcu_spi_receive(int port);
#endif
#ifdef CONFIG_I2C
int ti_arm_mcu_i2c_init(int port);
void ti_arm_mcu_i2c_reg_write(int port, unsigned char addr, unsigned char reg,
    unsigned char *data, int len);
#endif
#ifdef CONFIG_USB_DEVICE
void ti_arm_mcu_usb_set_addr(unsigned short addr);
void ti_arm_mcu_usb_ep_cfg(int ep, int max_pkt_size_in, int max_pkt_size_out,
    usb_ep_type_t type);
void ti_arm_mcu_usb_ep_data_ack(int ep, int data_phase);
int ti_arm_mcu_usb_ep_data_get(int ep, unsigned char *data, unsigned long len);
int ti_arm_mcu_usb_ep_data_send(int ep, unsigned char *data, unsigned long len,
    int last);
int ti_arm_mcu_usbd_event_process(void);
void ti_arm_mcu_usb_connect(void);
int ti_arm_mcu_usb_init(void);
#endif

#endif
