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
#ifndef __STM32_GPIO_H__
#define __STM32_GPIO_H__

/* XXX: perhaps this should be abstracted in each platform header file */
#if defined(CONFIG_STM32F3DISCOVERY)
#include "stm32f30x_rcc.h"
#include "stm32f30x_gpio.h"
#define PERIPH_ENABLE(p) RCC_AHBPeriphClockCmd(p, ENABLE)
#elif defined(CONFIG_STM32F4DISCOVERY)
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#define PERIPH_ENABLE(p) RCC_AHB1PeriphClockCmd(p, ENABLE)
#endif

#include "platform/platform.h"

typedef struct {
    unsigned long periph;
    GPIO_TypeDef *port;
} stm32_gpio_port_t;

extern const stm32_gpio_port_t stm32_gpio_ports[];

void stm32_gpio_digital_write(int pin, int value);
int stm32_gpio_set_pin_mode(int pin, gpio_pin_mode_t mode);
void stm32_gpio_set_port_val(int port, unsigned short value);

#endif
