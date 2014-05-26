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
#include "platform/platform.h"
#include "platform/arm/stm32/stm32.h"
#include "platform/arm/stm32/stm32_common.h"
#include "platform/arm/cortex-m.h"
#include "drivers/serial/serial_platform.h"

/* Define in each chip's system_xxx file */
extern uint32_t SystemCoreClock;

int stm32_select(int ms)
{
    int expire = platform_get_ticks_from_boot() + ms, event = 0;

    while ((!ms || platform_get_ticks_from_boot() < expire) && !event)
    {
        event |= buffered_serial_events_process();

        /* XXX: Sleep */
    }

    return event;
}

unsigned long stm32_get_system_clock(void)
{
    return SystemCoreClock;
}

void stm32_msleep(double ms)
{
    volatile unsigned long wait = (ms * SystemCoreClock) / 1000;

    while (wait--);
}

void stm32_init(void)
{
    SysTick_Config(SystemCoreClock / 1000);
}
