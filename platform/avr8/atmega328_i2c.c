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
#include <stdint.h>
#include <avr/io.h>
#include "platform/avr8/atmega328_i2c.h"

#define I2C_PORT PORTC
#define I2C_SCL PC5
#define I2C_SDA PC4

static void avr8_i2c_start(void)
{
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
}

static void avr8_i2c_stop(void)
{
    TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}

static void avr8_i2c_write(uint8_t c)
{
    TWDR = c;
    TWCR = (1<<TWINT) | (1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
}

int avr8_i2c_init(int port)
{
    TWBR = 0x03;
    TWSR = 0;
    TWCR |= 1<<TWEN;
 
    /* Enable pull-ups */
    I2C_PORT |= (1<<I2C_SCL)|(1<<I2C_SDA);
    return 0;
}

static inline uint8_t avr8_i2c_status(void)
{
    return TWSR & 0xF8;
}

void avr8_i2c_reg_write(int port, unsigned char addr, unsigned char reg,
    const unsigned char *data, int len)
{
#define EXPECT(x) if (avr8_i2c_status() != (x)) return
    avr8_i2c_start();
    EXPECT(0x08);
    avr8_i2c_write(addr);
    EXPECT(0x18);
    avr8_i2c_write(reg);
    EXPECT(0x28);
    while (len--)
    {
        avr8_i2c_write(*data++);
        EXPECT(0x28);
    }
    avr8_i2c_stop();
}
