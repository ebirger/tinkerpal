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
#ifndef __DRIVERS_GPIO_H__
#define __DRIVERS_GPIO_H__

#include "util/tstr.h"
#include "drivers/resources.h"
#include "platform/platform.h"

#define GPIO_RES(maj) RES(GPIO_RESOURCE_ID_BASE, maj, 0)

static inline void gpio_digital_write(resource_t pin, int value)
{
    if (RES_BASE(pin) != GPIO_RESOURCE_ID_BASE)
	return;

    platform.gpio.digital_write(RES_MAJ(pin), value);
}

static inline void gpio_digital_pulse(resource_t pin, int value, double ms)
{
    if (RES_BASE(pin) != GPIO_RESOURCE_ID_BASE)
	return;

    platform.gpio.digital_write(RES_MAJ(pin), value);
    platform_msleep(ms);
    platform.gpio.digital_write(RES_MAJ(pin), !value);
}

static inline int gpio_digital_read(resource_t pin)
{
    if (RES_BASE(pin) != GPIO_RESOURCE_ID_BASE)
	return 0;

    return platform.gpio.digital_read(RES_MAJ(pin));
}

static inline void gpio_analog_write(resource_t pin, double value)
{
    if (RES_BASE(pin) != GPIO_RESOURCE_ID_BASE)
	return;

    platform.gpio.analog_write(RES_MAJ(pin), value);
}

static inline double gpio_analog_read(resource_t pin)
{
    if (RES_BASE(pin) != GPIO_RESOURCE_ID_BASE)
	return 0;

    return platform.gpio.analog_read(RES_MAJ(pin));
}

static inline int gpio_set_pin_mode(resource_t pin, gpio_pin_mode_t mode)
{
    if (RES_BASE(pin) != GPIO_RESOURCE_ID_BASE)
	return -1;

    return platform.gpio.set_pin_mode(RES_MAJ(pin), mode);
}

int gpio_set_port_mode(resource_t port, u16 mask, gpio_pin_mode_t mode);

static inline void gpio_set_port_val(resource_t port, u16 mask, u16 value)
{
    if (RES_BASE(port) != GPIO_RESOURCE_ID_BASE)
	return;

    platform.gpio.set_port_val(RES_MAJ(port), mask, value);
}

static inline u16 gpio_get_port_val(resource_t port, u16 mask)
{
    if (RES_BASE(port) != GPIO_RESOURCE_ID_BASE)
	return 0;

    return platform.gpio.get_port_val(RES_MAJ(port), mask);
}

/* XXX: should receive tstr */
static inline int gpio_get_constant(int *constant, char *buf, int len)
{
    int pin;

#define GPIO_PREFIX "GPIO_P"

    if (len < sizeof(GPIO_PREFIX) -1 || 
	prefix_comp(sizeof(GPIO_PREFIX) - 1, GPIO_PREFIX, buf))
    {
	return -1;
    }

    buf += sizeof(GPIO_PREFIX) - 1;
    len -= sizeof(GPIO_PREFIX) - 1;

    if (len == 2)
	pin = GPIO(buf[0] - 'A', buf[1] - '0');
    else if (len == 3)
	pin = GPIO(buf[0] - 'A', ((buf[1] - '0') * 10) + buf[2] - '0');
    else 
	return -1;

    *constant = (int)RES(GPIO_RESOURCE_ID_BASE, pin, 0);
    return 0;
}

#endif
