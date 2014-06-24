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
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
#include "driverlib/sysctl.h"
#endif
#include "driverlib/adc.h"
#ifdef CONFIG_PLAT_HAS_PWM
#include "driverlib/pwm.h"
#endif
#include "drivers/gpio/gpio_platform.h"
#include "platform/arm/ti/ti_arm_mcu.h"

#ifdef CONFIG_PLAT_HAS_GPIO_INTERRUPTS
#define GPIO_INT_STATUS TI_BSP_IFDEF(MAP_GPIOPinIntStatus, MAP_GPIOIntStatus)
#define GPIO_INT_CLEAR TI_BSP_IFDEF(MAP_GPIOPinIntClear, MAP_GPIOIntClear)
#define GPIO_INT_DISABLE TI_BSP_IFDEF(MAP_GPIOPinIntDisable, MAP_GPIOIntDisable)
#define GPIO_INT_ENABLE TI_BSP_IFDEF(MAP_GPIOPinIntEnable, MAP_GPIOIntEnable)

void ti_arm_mcu_gpio_isr(int port)
{
    long istat;
    unsigned long base = ti_arm_mcu_gpio_ports[port].base;

    istat = GPIO_INT_STATUS(base, 1);
    GPIO_INT_CLEAR(base, istat);
    gpio_state_set(port, istat);
}
#endif

void ti_arm_mcu_gpio_input(int pin)
{
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
    unsigned long base = ti_arm_mcu_gpio_base(pin);
    int bit = GPIO_BIT(pin);

#ifdef CONFIG_PLAT_HAS_GPIO_INTERRUPTS
    GPIO_INT_DISABLE(base, bit);
#endif

    MAP_GPIODirModeSet(base, bit, GPIO_DIR_MODE_IN);
    MAP_GPIOPinTypeGPIOInput(base, bit);

#ifdef CONFIG_PLAT_HAS_GPIO_INTERRUPTS
    MAP_GPIOIntTypeSet(base, bit, GPIO_BOTH_EDGES);
    GPIO_INT_ENABLE(base, bit);
    MAP_IntEnable(ti_arm_mcu_gpio_ports[GPIO_PORT(pin)].irq);
#endif
#elif defined(CONFIG_CC3200)
    tp_crit(("GPIO input not implemented yet\n"));
#endif
}

#ifdef CONFIG_PLAT_HAS_PWM
static const ti_arm_mcu_pwm_t *pin_pwm(int pin)
{
    const ti_arm_mcu_pwm_t *pwm;

    for (pwm = ti_arm_mcu_pwms; pwm->base && pwm->pin != pin; pwm++);
    return pwm;
}

void ti_arm_mcu_gpio_pwm_start(int pin, int freq, int duty_cycle) 
{
    const ti_arm_mcu_pwm_t *pwm = pin_pwm(pin);
    unsigned long period, width;

    if (!pwm->base)
        return;
    
    /* Compute the PWM period based on the system clock */
    period = platform.get_system_clock() / freq;

    /* Set the PWM period  */
    MAP_PWMGenConfigure(pwm->base, pwm->gen, PWM_GEN_MODE_UP_DOWN |
        PWM_GEN_MODE_NO_SYNC);
    MAP_PWMGenPeriodSet(pwm->base, pwm->gen, period);

    width = (unsigned long)(duty_cycle * period / 100);
    /* Taking up too much of the period will result in having nothing */
    if (width > period - 5)
        width = period - 5;

    /* Set PWM0 duty cycle */
    MAP_PWMPulseWidthSet(pwm->base, pwm->out, width);

    /* Enable the PWM0 output signals */
    MAP_PWMOutputState(pwm->base, pwm->out_bit, true);

    /* Enable the PWM generator */
    MAP_PWMGenEnable(pwm->base, pwm->gen);
}

int ti_arm_mcu_pin_mode_pwm(int pin)
{
    const ti_arm_mcu_pwm_t *pwm = pin_pwm(pin);

    if (!pwm->base)
        return -1;

    MAP_SysCtlPeripheralEnable(pwm->periph);
    if (pwm->af)
        MAP_GPIOPinConfigure(pwm->af);
    MAP_GPIOPinTypePWM(ti_arm_mcu_gpio_base(pin), GPIO_BIT(pin));
    return 0;
}
#endif

void ti_arm_mcu_gpio_digital_write(int pin, int value)
{
    MAP_GPIOPinWrite(ti_arm_mcu_gpio_base(pin), GPIO_BIT(pin), 
        value ? GPIO_BIT(pin) : 0);
}

int ti_arm_mcu_gpio_digital_read(int pin)
{
    return MAP_GPIOPinRead(ti_arm_mcu_gpio_base(pin), GPIO_BIT(pin)) ? 1 : 0;
}

double ti_arm_mcu_gpio_analog_read(int pin) 
{
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
    int channel, value;

    channel = ti_arm_mcu_gpio_pins[pin].adc_channel;
    if (channel == -1) 
        return 0;

    MAP_ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_IE | ADC_CTL_END | 
        channel);
    MAP_ADCSequenceEnable(ADC0_BASE, 0);

    MAP_ADCProcessorTrigger(ADC0_BASE, 0);

    while (!MAP_ADCIntStatus(ADC0_BASE, 0, 0));

    MAP_ADCSequenceDataGet(ADC0_BASE, 0, (unsigned long *)&value);
    return (double)value / 4096;
#elif defined(CONFIG_CC3200)
    tp_crit(("GPIO input not implemented yet\n"));
    return 0;
#endif
}

void ti_arm_mcu_gpio_set_port_val(int port, unsigned short mask,
    unsigned short value)
{
    MAP_GPIOPinWrite(ti_arm_mcu_gpio_port_base(port), mask,
        (unsigned char)value);
}

unsigned short ti_arm_mcu_gpio_get_port_val(int port, unsigned short mask)
{
    return MAP_GPIOPinRead(ti_arm_mcu_gpio_port_base(port), mask);
}
