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
#ifndef __TM4C1294_CONSTS_H__
#define __TM4C1294_CONSTS_H__

#define GPIO(port, pin) (((port) << 3) + (pin))
#define GPIO_BIT(p) (1 << ((p) & 0x7))
#define GPIO_PORT(p) ((p) >> 3)
#define GPIO_NUM_PORT_PINS 8

typedef char gpio_port_t;

#define TIMER0 0
#define TIMER1 1
#define TIMER2 2
#define TIMER3 3
#define TIMER4 4
#define TIMER5 5
#define WTIMER0 6
#define WTIMER1 7
#define WTIMER2 8
#define WTIMER3 9
#define WTIMER4 10
#define WTIMER5 11

#define UART0 0
#define UART1 1
#define UART2 2
#define UART3 3
#define UART4 4
#define UART5 5
#define UART6 6
#define UART7 7
#define NUM_UARTS 8

#define SSI0 0
#define SSI1 1
#define SSI2 2

#define I2C0 0
#define I2C1 1
#define I2C2 2
#define I2C3 3
#define I2C4 4
#define I2C5 5
#define I2C6 6
#define I2C7 7
#define I2C8 8
#define I2C9 9

#define PA0 0x00 
#define PA1 0x01 
#define PA2 0x02 
#define PA3 0x03 
#define PA4 0x04 
#define PA5 0x05 
#define PA6 0x06 
#define PA7 0x07
#define PB0 0x08
#define PB1 0x09
#define PB2 0x0a
#define PB3 0x0b
#define PB4 0x0c
#define PB5 0x0d
#define PB6 0x0e
#define PB7 0x0f
#define PC0 0x10
#define PC1 0x11
#define PC2 0x12
#define PC3 0x13
#define PC4 0x14
#define PC5 0x15
#define PC6 0x16
#define PC7 0x17
#define PD0 0x18
#define PD1 0x19
#define PD2 0x1a
#define PD3 0x1b
#define PD4 0x1c
#define PD5 0x1d
#define PD6 0x1e
#define PD7 0x1f
#define PE0 0x20
#define PE1 0x21
#define PE2 0x22
#define PE3 0x23
#define PE4 0x24
#define PE5 0x25
#define PE6 0x26
#define PE7 0x27
#define PF0 0x28
#define PF1 0x29
#define PF2 0x2a
#define PF3 0x2b
#define PF4 0x2c
#define PF5 0x2d
#define PF6 0x2e
#define PF7 0x2f
#define PG0 0x30
#define PG1 0x31
#define PG2 0x32
#define PG3 0x33
#define PG4 0x34
#define PG5 0x35
#define PG6 0x36
#define PG7 0x37
#define PH0 0x38
#define PH1 0x39
#define PH2 0x3a
#define PH3 0x3b
#define PH4 0x3c
#define PH5 0x3d
#define PH6 0x3e
#define PH7 0x3f
#define PJ0 0x40
#define PJ1 0x41
#define PJ2 0x42
#define PJ3 0x43
#define PJ4 0x44
#define PJ5 0x45
#define PJ6 0x46
#define PJ7 0x47
#define PK0 0x48
#define PK1 0x49
#define PK2 0x4a
#define PK3 0x4b
#define PK4 0x4c
#define PK5 0x4d
#define PK6 0x4e
#define PK7 0x4f
#define PL0 0x50
#define PL1 0x51
#define PL2 0x52
#define PL3 0x53
#define PL4 0x54
#define PL5 0x55
#define PL6 0x56
#define PL7 0x57
#define PM0 0x58
#define PM1 0x59
#define PM2 0x5a
#define PM3 0x5b
#define PM4 0x5c
#define PM5 0x5d
#define PM6 0x5e
#define PM7 0x5f
#define PN0 0x60
#define PN1 0x61
#define PN2 0x62
#define PN3 0x63
#define PN4 0x64
#define PN5 0x65
#define PN6 0x66
#define PN7 0x67
#define PP0 0x68
#define PP1 0x69
#define PP2 0x6a
#define PP3 0x6b
#define PP4 0x6c
#define PP5 0x6d
#define PP6 0x6e
#define PP7 0x6f
#define PQ0 0x70
#define PQ1 0x71
#define PQ2 0x72
#define PQ3 0x73
#define PQ4 0x74
#define PQ5 0x75
#define PQ6 0x76
#define PQ7 0x77

#define GPIO_PORT_A 0
#define GPIO_PORT_B 1
#define GPIO_PORT_C 2
#define GPIO_PORT_D 3
#define GPIO_PORT_E 4
#define GPIO_PORT_F 5
#define GPIO_PORT_G 6
#define GPIO_PORT_H 7
#define GPIO_PORT_J 8
#define GPIO_PORT_K 9
#define GPIO_PORT_L 10
#define GPIO_PORT_M 11
#define GPIO_PORT_N 12
#define GPIO_PORT_P 13
#define GPIO_PORT_Q 14
#define NUM_GPIO_PORTS 15

#endif
