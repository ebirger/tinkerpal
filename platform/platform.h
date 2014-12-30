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

#include <stdint.h>
#include "util/debug.h"
#include "drivers/serial/serial_platform.h"
#include "usb/usbd_core_platform.h"
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
    serial_driver_t serial;
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
        void (*pwm_start)(int pin, int freq, int duty_cycle);
        double (*analog_read)(int pin);
        int (*set_pin_mode)(int pin, gpio_pin_mode_t mode);
        void (*set_port_val)(int port, unsigned short mask,
            unsigned short value);
        unsigned short (*get_port_val)(int port, unsigned short mask);
    } gpio;
    struct {
        int (*init)(int port);
        /* reconf - set all pins to SPI mode */
        void (*reconf)(int port);
        void (*set_max_speed)(int port, unsigned long speed);
        void (*send)(int port, unsigned long data);
        unsigned long (*receive)(int port);
    } spi;
    struct {
        int (*init)(int port);
        void (*reg_write)(int port, unsigned char addr, unsigned char reg,
            const unsigned char *data, int len);
    } i2c;
    struct {
        int (*init)(void);
        void (*connect)(void);
        void (*ep_cfg)(int ep, int max_pkt_size_in, int max_pkt_size_out,
            usb_ep_type_t type);
        void (*ep_data_ack)(int ep, int data_phase);
        int (*ep_data_wait)(int ep, unsigned char *data, unsigned long len);
        int (*ep_data_get)(int ep, unsigned char *data, unsigned long len);
        int (*ep_data_send)(int ep, const unsigned char *data,
            unsigned long len, int last);
        void (*set_addr)(unsigned short addr);
    } usb;
    void (*init)(void);
    void (*meminfo)(void);
    void (*get_time_from_boot)(uint32_t *sec, uint32_t *usec);
    unsigned long (*get_system_clock)(void);
    void (*msleep)(double ms);
    int (*select)(int ms);
    void (*panic)(void);
} platform_t;

#define platform_init() platform.init()
static inline void platform_uninit(void) { }

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

static inline void platform_get_time_from_boot(uint32_t *sec, uint32_t *usec)
{
    tp_assert(platform.get_time_from_boot);
    platform.get_time_from_boot(sec, usec);
}

static inline uint64_t platform_get_ticks_from_boot(void)
{
    uint32_t sec, usec;

    platform_get_time_from_boot(&sec, &usec);
    return (uint64_t)sec * 1000 + usec / 1000;
}

static inline void platform_msleep(double ms)
{
    tp_assert(platform.msleep);
    platform.msleep(ms);
}

#endif
