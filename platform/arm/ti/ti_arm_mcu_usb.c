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
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/usb.h"
#include "platform/platform.h"
#include "platform/arm/ti/ti_arm_mcu.h"

static unsigned long ctrl_istat, endp_istat;

int ti_arm_mcu_usb_ep0_data_get(unsigned char *data, unsigned long len)
{
    int rc;

    rc = MAP_USBEndpointDataGet(USB0_BASE, USB_EP_0, data, &len);
    if (rc)
        return -1;

    return len;
}

int ti_arm_mcu_usbd_event_process(void)
{
    MAP_IntDisable(INT_USB0);
    if (!ctrl_istat && !endp_istat)
    {
        MAP_IntEnable(INT_USB0);
        return 0;
    }

    tp_out(("%s: called %x %x\n", __FUNCTION__, ctrl_istat, endp_istat));

    ctrl_istat = endp_istat = 0;
    MAP_IntEnable(INT_USB0);
    return 0;
}

void ti_arm_mcu_usb_isr(void)
{
    ctrl_istat |= MAP_USBIntStatusControl(USB0_BASE);
    endp_istat |= MAP_USBIntStatusEndpoint(USB0_BASE);
    tp_out(("%s: called %x %x\n", __FUNCTION__, ctrl_istat, endp_istat));
}

static inline void ti_arm_mcu_pin_mode_usb(int pin)
{
    ti_arm_mcu_periph_enable(ti_arm_mcu_gpio_periph(pin));
    MAP_GPIOPinTypeUSBAnalog(ti_arm_mcu_gpio_base(pin), GPIO_BIT(pin));
}
int ti_arm_mcu_usb_init(void)
{
    uint32_t speed;

    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_USB0);
    ti_arm_mcu_periph_enable(SYSCTL_PERIPH_USB0);
    ti_arm_mcu_pin_mode_usb(ti_arm_mcu_usbd_params.dp_pin);
    ti_arm_mcu_pin_mode_usb(ti_arm_mcu_usbd_params.dm_pin);

    MAP_SysCtlUSBPLLEnable();
    MAP_USBClockEnable(USB0_BASE, 8, USB_CLOCK_INTERNAL);

    MAP_USBULPIDisable(USB0_BASE);

    MAP_USBDevMode(USB0_BASE);

    MAP_USBDevLPMDisable(USB0_BASE);
    MAP_USBDevLPMConfig(USB0_BASE, USB_DEV_LPM_NONE);

    /* Clear pending interrupts */
    MAP_USBIntStatusControl(USB0_BASE);
    MAP_USBIntStatusEndpoint(USB0_BASE);

    MAP_USBIntEnableControl(USB0_BASE, USB_INTCTRL_RESET);
    MAP_USBIntEnableEndpoint(USB0_BASE, USB_INTEP_0);

    MAP_USBDevConnect(USB0_BASE);
    MAP_IntEnable(INT_USB0);
    tp_out(("USB controller version 0x%x\n", USBControllerVersion(USB0_BASE)));
    tp_out(("Number of endpoints %d\n", USBNumEndpointsGet(USB0_BASE)));
    speed = USBDevSpeedGet(USB0_BASE);
    tp_out(("USB %s speed\n",
        speed == USB_HIGH_SPEED ? "high" :
        speed == USB_FULL_SPEED ? "full" : 
        speed == USB_LOW_SPEED ? "low" :
        "unknown"));
    return 0;
}
