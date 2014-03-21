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
#include "inc/hw_types.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "platform/platform.h"
#include "platform/arm/ti/ti_arm_mcu.h"

static inline unsigned long ti_arm_mcu_ssi_base(int port)
{
    return ti_arm_mcu_ssis[port].base;
}

static inline unsigned long ti_arm_mcu_ssi_periph(int port)
{
    return ti_arm_mcu_ssis[port].periph;
}

void ti_arm_mcu_spi_set_max_speed(int port, unsigned long speed)
{
    MAP_SSIDisable(ti_arm_mcu_ssi_base(port));

    /* Set the maximum speed as half the system clock, with a max of 12.5 MHz. */
    if (speed > 12500000)
       speed = 12500000;

    /* Configure the SSI port */
    /* XXX: transfer length should be configured by init */
    MAP_SSIConfigSetExpClk(ti_arm_mcu_ssi_base(port), MAP_SysCtlClockGet(),
	SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, speed, 8);

    MAP_SSIEnable(ti_arm_mcu_ssi_base(port));
}

void ti_arm_mcu_spi_send(int port, unsigned long data)
{
    unsigned long dummy;

    MAP_SSIDataPut(ti_arm_mcu_ssi_base(port), data); /* Write the data to the tx fifo */
    MAP_SSIDataGet(ti_arm_mcu_ssi_base(port), &dummy); /* flush data read during write */
}

unsigned long ti_arm_mcu_spi_receive(int port)
{
    unsigned long data;

    MAP_SSIDataPut(ti_arm_mcu_ssi_base(port), 0xFF); /* write dummy data */
    MAP_SSIDataGet(ti_arm_mcu_ssi_base(port), &data); /* read data frm rx fifo */
    return data;
}

void ti_arm_mcu_spi_reconf(int port)
{
    ti_arm_mcu_periph_enable(ti_arm_mcu_ssi_periph(port));

    /* Configure the appropriate pins to be SSI instead of GPIO */
    ti_arm_mcu_pin_mode_ssi(ti_arm_mcu_ssis[port].clk);
    ti_arm_mcu_pin_mode_ssi(ti_arm_mcu_ssis[port].rx);
    ti_arm_mcu_pin_mode_ssi(ti_arm_mcu_ssis[port].tx);
}

int ti_arm_mcu_spi_init(int port)
{
    ti_arm_mcu_spi_reconf(port);

    /* Configure the SSI port and enable the port */
    ti_arm_mcu_spi_set_max_speed(port, 400000); 
    return 0;
}
