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
#ifndef __STM32F4DISCOVERY_CONSTS_H__
#define __STM32F4DISCOVERY_CONSTS_H__

#define GPIO(port, pin) (((port) << 4) + (pin))
#define GPIO_BIT(p) (1 << ((p) & 0xf))
#define GPIO_NUM_PORT_PINS 16

typedef short gpio_port_t;

#define GPIO_PORT_A 0
#define GPIO_PORT_B 1
#define GPIO_PORT_C 2
#define GPIO_PORT_D 3
#define GPIO_PORT_E 4
#define GPIO_PORT_F 5
#define NUM_GPIO_PORTS 6

#define USART_PORT1 0
#define USART_PORT2 1
#define USART_PORT3 2
#define UART_PORT4 3
#define UART_PORT5 4
#define USART_PORT6 5
#define NUM_UARTS 6

#define SPI_PORT1 0
#define SPI_PORT2 1
#define SPI_PORT3 2

#define PA0 0x00
#define PA1 0x01
#define PA2 0x02
#define PA3 0x03
#define PA4 0x04
#define PA5 0x05
#define PA6 0x06
#define PA7 0x07
#define PA8 0x08
#define PA9 0x09 
#define PA10 0x0a
#define PA11 0x0b
#define PA12 0x0c
#define PA13 0x0d
#define PA14 0x0e
#define PA15 0x0f

#define PB0 0x10
#define PB1 0x11
#define PB2 0x12
#define PB3 0x13
#define PB4 0x14
#define PB5 0x15
#define PB6 0x16
#define PB7 0x17
#define PB8 0x18
#define PB9 0x19 
#define PB10 0x1a
#define PB11 0x1b
#define PB12 0x1c
#define PB13 0x1d
#define PB14 0x1e
#define PB15 0x1f

#define PC0 0x20
#define PC1 0x21
#define PC2 0x22
#define PC3 0x23
#define PC4 0x24
#define PC5 0x25
#define PC6 0x26
#define PC7 0x27
#define PC8 0x28
#define PC9 0x29 
#define PC10 0x2a
#define PC11 0x2b
#define PC12 0x2c
#define PC13 0x2d
#define PC14 0x2e
#define PC15 0x2f

#define PD0 0x30
#define PD1 0x31
#define PD2 0x32
#define PD3 0x33
#define PD4 0x34
#define PD5 0x35
#define PD6 0x36
#define PD7 0x37
#define PD8 0x38
#define PD9 0x39 
#define PD10 0x3a
#define PD11 0x3b
#define PD12 0x3c
#define PD13 0x3d
#define PD14 0x3e
#define PD15 0x3f

#define PE0 0x40
#define PE1 0x41
#define PE2 0x42
#define PE3 0x43
#define PE4 0x44
#define PE5 0x45
#define PE6 0x46
#define PE7 0x47
#define PE8 0x48
#define PE9 0x49 
#define PE10 0x4a
#define PE11 0x4b
#define PE12 0x4c
#define PE13 0x4d
#define PE14 0x4e
#define PE15 0x4f

#define PF0 0x50
#define PF1 0x51
#define PF2 0x52
#define PF3 0x53
#define PF4 0x54
#define PF5 0x55
#define PF6 0x56
#define PF7 0x57
#define PF8 0x58
#define PF9 0x59 
#define PF10 0x5a
#define PF11 0x5b
#define PF12 0x5c
#define PF13 0x5d
#define PF14 0x5e
#define PF15 0x5f

#endif
