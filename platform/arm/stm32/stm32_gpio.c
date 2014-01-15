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
#include "platform/arm/stm32/stm32_gpio.h"
#include "drivers/gpio/gpio_platform.h"

#define GPIO_PERIPH(p) (stm32_gpio_ports[((p) >> 4)].periph)
#define GPIO_PORT(p) (stm32_gpio_ports[((p) >> 4)].port)

void stm32_gpio_digital_write(int pin, int value)
{
    if (value)
	GPIO_SetBits(GPIO_PORT(pin), GPIO_BIT(pin));
    else
	GPIO_ResetBits(GPIO_PORT(pin), GPIO_BIT(pin));
}

void stm32_gpio_set_port_val(int port, unsigned short value)
{
    GPIO_ResetBits(stm32_gpio_ports[port].port, ~value);
    GPIO_SetBits(stm32_gpio_ports[port].port, value);
}

void stm32_gpio_set_pin_function(int pin, uint8_t af)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_PinAFConfig(GPIO_PORT(pin), pin & (GPIO_NUM_PORT_PINS - 1), af);

    GPIO_InitStructure.GPIO_Pin = GPIO_BIT(pin); 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIO_PORT(pin), &GPIO_InitStructure);
}

int stm32_gpio_set_pin_mode(int pin, gpio_pin_mode_t mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    PERIPH_ENABLE(GPIO_PERIPH(pin));

    switch (mode)
    {
    case GPIO_PM_OUTPUT:
	/* XXX: not all pins are actually available */
	GPIO_InitStructure.GPIO_Pin = GPIO_BIT(pin); 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIO_PORT(pin), &GPIO_InitStructure);
	break;
    default:
	tp_err(("Pinmode %d is not supported yet\n", mode));
	return -1;
    }
    return 0;
}
