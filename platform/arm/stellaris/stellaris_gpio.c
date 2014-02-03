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
#include "driverlib/adc.h"
#include "drivers/gpio/gpio_platform.h"
#include "platform/arm/stellaris/stellaris.h"

#ifdef CONFIG_PLAT_HAS_GPIO_INTERRUPTS
void stellaris_gpio_isr(int port)
{
    long istat;
    unsigned long base = stellaris_gpio_ports[port].base;

    istat = MAP_GPIOPinIntStatus(base, 1);
    MAP_GPIOPinIntClear(base, istat);
    gpio_state_set(port, istat);
}
#endif

void stellaris_gpio_input(int pin)
{
    unsigned long base = stellaris_gpio_base(pin);
    int bit = GPIO_BIT(pin);

#ifdef CONFIG_PLAT_HAS_GPIO_INTERRUPTS
    MAP_GPIOPinIntDisable(base, bit);
#endif

    MAP_GPIODirModeSet(base, bit, GPIO_DIR_MODE_IN);
    MAP_GPIOPinTypeGPIOInput(base, bit);

#ifdef CONFIG_PLAT_HAS_GPIO_INTERRUPTS
    MAP_GPIOIntTypeSet(base, bit, GPIO_BOTH_EDGES);
    MAP_GPIOPinIntEnable(base, bit);
    MAP_IntEnable(stellaris_gpio_ports[GPIO_PORT(pin)].irq);
#endif
}

void stellaris_gpio_digital_write(int pin, int value)
{
    MAP_GPIOPinWrite(stellaris_gpio_base(pin), GPIO_BIT(pin), 
	value ? GPIO_BIT(pin) : 0);
}

int stellaris_gpio_digital_read(int pin)
{
    return MAP_GPIOPinRead(stellaris_gpio_base(pin), GPIO_BIT(pin)) ? 1 : 0;
}

double stellaris_gpio_analog_read(int pin) 
{
    int channel, value;

    channel = stellaris_gpio_pins[pin].adc_channel;
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
}

void stellaris_gpio_set_port_val(int port, unsigned short mask,
    unsigned short value)
{
    MAP_GPIOPinWrite(stellaris_gpio_port_base(port), mask,
	(unsigned char)value);
}

unsigned short stellaris_gpio_get_port_val(int port, unsigned short mask)
{
    return MAP_GPIOPinRead(stellaris_gpio_port_base(port), mask);
}
