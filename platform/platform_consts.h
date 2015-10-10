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
#ifndef __PLATFORM_CONSTS_H__
#define __PLATFORM_CONSTS_H__

#ifdef CONFIG_PLATFORM_EMULATION
#include "platform/unix/sim.h"
#elif defined(CONFIG_LM4F120XL)
#include "platform/arm/ti/lm4f120xl/lm4f120xl.h"
#elif defined(CONFIG_LM3S6965)
#include "platform/arm/ti/lm3s6965/lm3s6965.h"
#elif defined(CONFIG_LM3S6918)
#include "platform/arm/ti/lm3s6918/lm3s6918.h"
#elif defined(CONFIG_TM4C123G)
#include "platform/arm/ti/tm4c123g/tm4c123g.h"
#elif defined(CONFIG_TM4C1294)
#include "platform/arm/ti/tm4c1294/tm4c1294.h"
#elif defined(CONFIG_CC3200)
#include "platform/arm/ti/cc3200/cc3200.h"
#elif defined(CONFIG_STM32F103XX)
#include "platform/arm/stm32/stm32f1xx/stm32f103xx.h"
#elif defined(CONFIG_STM32F303XX)
#include "platform/arm/stm32/stm32f3xx/stm32f303xx.h"
#elif defined(CONFIG_STM32F407XX)
#include "platform/arm/stm32/stm32f4xx/stm32f407xx.h"
#elif defined(CONFIG_STM32F429XX)
#include "platform/arm/stm32/stm32f4xx/stm32f429xx.h"
#elif defined(CONFIG_FRDM_KL25Z)
#include "platform/arm/frdm/kl25z.h"
#elif defined(CONFIG_MSP430F5529)
#include "platform/msp430/msp430f5529.h"
#elif defined(CONFIG_ATMEGA328)
#include "platform/avr8/atmega328.h"
#elif defined(CONFIG_ESP8266)
#include "platform/esp8266/esp8266.h"
#elif defined(CONFIG_X86_PLATFORM_EMULATION)
#else
#error Platform constants not defined
#endif

#endif
