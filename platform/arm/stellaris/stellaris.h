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
#ifndef __STERLLARIS_H__
#define __STERLLARIS_H__

#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

typedef struct {
    unsigned long periph;
    unsigned long base;
    unsigned long irq;
    int rxpin;
    int txpin;
} stellaris_uart_t;

typedef struct {
    unsigned long periph;
    unsigned long base;
    int irq;
} stellaris_gpio_port_t;

typedef struct {
    unsigned long periph;
    unsigned long base;
    int clk;
    int fss;
    int rx;
    int tx;
} stellaris_ssi_t;

typedef struct {
    unsigned long periph;
    unsigned long base;
} stellaris_timer_t;

typedef struct {
    int timer;
    int timer_function;
    int adc_channel;
    int uart_function;
    int ssi_function;
} stellaris_gpio_pin_t;

/* Defined in each specific target board */
extern const stellaris_uart_t stellaris_uarts[];
extern const stellaris_gpio_port_t stellaris_gpio_ports[];
extern const stellaris_ssi_t stellaris_ssis[];
extern const stellaris_timer_t stellaris_timers[];
extern const stellaris_gpio_pin_t stellaris_gpio_pins[];

void stellaris_systick_init(void);
int stellaris_get_system_clock(void);
void stellaris_msleep(double ms);

void stellaris_uart_enable(int u, int enabled);
int stellaris_select(int ms);
void stellaris_serial_irq_enable(int u, int enable);
int stellaris_serial_write(int u, char *buf, int size);

static inline void stellaris_periph_enable(unsigned long periph)
{
    MAP_SysCtlPeripheralEnable(periph);
    MAP_SysCtlPeripheralSleepEnable(periph);
}

static inline unsigned long stellaris_gpio_periph(int pin)
{
    return stellaris_gpio_ports[GPIO_PORT(pin)].periph;
}

static inline unsigned long stellaris_gpio_base(int pin)
{
    return stellaris_gpio_ports[GPIO_PORT(pin)].base;
}

static inline void stellaris_pin_mode_output(int pin)
{
    MAP_GPIOPinTypeGPIOOutput(stellaris_gpio_base(pin), GPIO_BIT(pin));
}

static inline void stellaris_pin_config(int pin, int mode)
{
    MAP_GPIOPadConfigSet(stellaris_gpio_base(pin), GPIO_BIT(pin), 
	GPIO_STRENGTH_8MA, mode);
}

static inline void stellaris_pin_mode_ssi(int pin)
{
    stellaris_periph_enable(stellaris_gpio_periph(pin));
    if (stellaris_gpio_pins[pin].ssi_function != -1)
	MAP_GPIOPinConfigure(stellaris_gpio_pins[pin].ssi_function);
    MAP_GPIOPinTypeSSI(stellaris_gpio_base(pin), GPIO_BIT(pin));
    stellaris_pin_config(pin, GPIO_PIN_TYPE_STD_WPU);
}

static inline int stellaris_pin_mode_adc(int pin)
{
    if (stellaris_gpio_pins[pin].adc_channel == -1)
	return -1;

    stellaris_periph_enable(SYSCTL_PERIPH_ADC0);
    MAP_GPIOPinTypeADC(stellaris_gpio_base(pin), GPIO_BIT(pin));
    return 0;
}

static inline void stellaris_pin_mode_timer(int pin)
{
    int timer_function;

    MAP_GPIODirModeSet(stellaris_gpio_base(pin), GPIO_BIT(pin), 
	GPIO_DIR_MODE_HW);
    stellaris_pin_config(pin, GPIO_PIN_TYPE_STD);
    MAP_GPIOPinTypeTimer(stellaris_gpio_base(pin), GPIO_BIT(pin));
    if ((timer_function = stellaris_gpio_pins[pin].timer_function) != -1)
	MAP_GPIOPinConfigure(timer_function);
}

#ifdef CONFIG_GPIO
void stellaris_gpio_input(int pin);
void stellaris_gpio_digital_write(int pin, int value);
int stellaris_gpio_digital_read(int pin);
double stellaris_gpio_analog_read(int pin);
#endif
#ifdef CONFIG_SPI
int stellaris_spi_init(int port);
void stellaris_spi_reconf(int port);
void stellaris_spi_set_max_speed(int port, int speed);
void stellaris_spi_send(int port, unsigned long data);
unsigned long stellaris_spi_receive(int port);
#endif

#endif
