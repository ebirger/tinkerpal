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
#include "drivers/gpio/gpio.h"
#include "drivers/gpio/gpio_platform.h"
#include "util/event.h"

gpio_port_t gpio_int_state[NUM_GPIO_PORTS] = {};

static int gpio_event(int port)
{
    int ret = 0, i;
    gpio_port_t state;

    /* XXX: critical section here, disable port interrupts */
    state = gpio_int_state[port];
    gpio_int_state[port] = 0;
    /* XXX: end critical section here, enable port interrupts */

    for (i = 0; i < GPIO_NUM_PORT_PINS; i++)
    {
	if (state & GPIO_BIT(i))
	{
	    u32 res = RES(GPIO_RESOURCE_ID_BASE, GPIO(port, i), 0);

	    event_watch_trigger(res);
	    ret = 1;
	}
    }
    return ret;
}

int gpio_events_process(void)
{
    int i, event = 0;

    for (i = 0; i < NUM_GPIO_PORTS; i++)
    {
	if (gpio_int_state[i])
	    event |= gpio_event(i);
    }
    return event;
}

int gpio_set_port_mode(resource_t port, gpio_pin_mode_t mode)
{
    u16 pin;

    if (RES_BASE(port) != GPIO_RESOURCE_ID_BASE)
	return -1;

    for (pin = 0; pin < GPIO_NUM_PORT_PINS; pin++)
    {
	if (gpio_set_pin_mode(GPIO_RES(GPIO(RES_MAJ(port), pin)), mode))
	    return -1;
    }

    return 0;
}
