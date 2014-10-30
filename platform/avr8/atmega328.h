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
#ifndef __ATMEGA328_CONSTS_H__
#define __ATMEGA328_CONSTS_H__

#define GPIO(port, pin) (((port) << 3) + (pin))
#define GPIO_BIT(p) (1 << ((p) & 0x7))
#define GPIO_PORT(p) ((p) >> 3)
#define GPIO_NUM_PORT_PINS 8

typedef char gpio_port_t;

#define NUM_UARTS 1
#define USART0 0

#define NUM_GPIO_PORTS 4

#define GPIO_PORT_A 0
#define GPIO_PORT_B 1
#define GPIO_PORT_C 2
#define GPIO_PORT_D 3

#define _PA0 0x00
#define _PA1 0x01
#define _PA2 0x02
#define _PA3 0x03
#define _PA4 0x04
#define _PA5 0x05
#define _PA6 0x06
#define _PA7 0x07
#define _PB0 0x08
#define _PB1 0x09
#define _PB2 0x0a
#define _PB3 0x0b
#define _PB4 0x0c
#define _PB5 0x0d
#define _PB6 0x0e
#define _PB7 0x0f
#define _PC0 0x10
#define _PC1 0x11
#define _PC2 0x12
#define _PC3 0x13
#define _PC4 0x14
#define _PC5 0x15
#define _PC6 0x16
#define _PC7 0x17
#define _PD0 0x18
#define _PD1 0x19
#define _PD2 0x1a
#define _PD3 0x1b
#define _PD4 0x1c
#define _PD5 0x1d
#define _PD6 0x1e
#define _PD7 0x1f

#endif
