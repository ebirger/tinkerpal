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
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "util/debug.h"
#include "platform/platform_consts.h"

typedef enum {
    GPIO_PM_INPUT = 0,
    GPIO_PM_OUTPUT = 1,
    GPIO_PM_INPUT_PULLUP = 2,
    GPIO_PM_INPUT_PULLDOWN = 3,
    GPIO_PM_INPUT_ANALOG = 4,
    GPIO_PM_OUTPUT_ANALOG = 5,
} gpio_pin_mode_t;

typedef struct {
    char *desc;
    struct {
	int (*enable)(int u, int enabled);
	int (*read)(int u, char *buf, int size);
	int (*write)(int u, char *buf, int size);
	void (*irq_enable)(int u, int enable);
	int default_console_id;
    } serial;
    struct {
	int (*init)(void);
	int (*status)(void);
	int (*ioctl)(int cmd, void *buf);
	int (*read)(unsigned char *buf, int sector, int count);
	int (*write)(const unsigned char *buf, int sector, int count);
    } block;
    struct {
	void (*digital_write)(int pin, int value);
	int (*digital_read)(int pin);
	void (*analog_write)(int pin, double value);
	double (*analog_read)(int pin);
	int (*set_pin_mode)(int pin, gpio_pin_mode_t mode);
    } gpio;
    struct {
	int (*init)(int port);
	/* reconf - set all pins to SPI mode */
	void (*reconf)(int port);
	void (*set_max_speed)(int port, int speed);
	void (*send)(int port, unsigned long data);
	unsigned long (*receive)(int port);
    } spi;
    void (*init)(void);
    void (*meminfo)(void);
    int (*get_ticks_from_boot)(void);
    int (*get_system_clock)(void);
    void (*msleep)(double ms);
    int (*select)(int ms, int (*is_active)(int id), void (*mark_on)(int id));
    void (*panic)(void);
} platform_t;

#define platform_init() platform.init()
#define platform_uninit() do { } while(0)

/* The platform global is defined in each platform,
 * it's kinda icky, but I would like to avoid the pointer
 * dereference on every platform API access.
 */
extern const platform_t platform;

static inline void platform_meminfo(void)
{
    if (!platform.meminfo)
    {
	tp_warn(("No platform meminfo available\n"));
	return;
    }
    platform.meminfo();
}

#endif
