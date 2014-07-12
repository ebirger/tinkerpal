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
#include "usb/usbd_core_platform.h"

static unsigned long ctrl_istat, endp_istat;

static uint32_t ep_map(int ep)
{
    switch (ep)
    {
    case USBD_EP0: return USB_EP_0;
    case USBD_EP1: return USB_EP_1;
    case USBD_EP2: return USB_EP_2;
    }
    return 0;
}

void ti_arm_mcu_usb_set_addr(unsigned short addr)
{
    MAP_USBDevAddrSet(USB0_BASE, addr);
}

void ti_arm_mcu_usb_ep_data_ack(int ep, int data_phase)
{
    USBDevEndpointDataAck(USB0_BASE, ep_map(ep), data_phase ? false : true);
}

int ti_arm_mcu_usb_ep_data_send(int ep, unsigned char *data, unsigned long len,
    int last)
{
    uint32_t mapped_ep = ep_map(ep);

    tp_out(("%s: len %d\n", __FUNCTION__, len));
    if (len)
    {
        if (MAP_USBEndpointDataPut(USB0_BASE, mapped_ep, data, len))
            return -1;
    }
    
    return MAP_USBEndpointDataSend(USB0_BASE, mapped_ep, last ?
        USB_TRANS_IN_LAST : USB_TRANS_IN);
}

int ti_arm_mcu_usb_ep_data_get(int ep, unsigned char *data, unsigned long len)
{
    if (MAP_USBEndpointDataGet(USB0_BASE, ep_map(ep), data, &len))
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

    if (ctrl_istat & USB_INTCTRL_RESET)
        usbd_event(0, USB_DEVICE_EVENT_RESET);
    if (endp_istat & USB_INTEP_0)
    {
        if (MAP_USBEndpointStatus(USB0_BASE, USB_EP_0) & USB_DEV_EP0_OUT_PKTRDY)
            usbd_event(USBD_EP0, USB_DEVICE_EVENT_EP_DATA_READY);
        else
        {
            /* XXX: endpoint may be at fault, need to check */
            usbd_event(USBD_EP0, USB_DEVICE_EVENT_EP_WRITE_ACK);
        }
    }
    if (endp_istat & USB_INTEP_DEV_OUT_1)
        usbd_event(USBD_EP1, USB_DEVICE_EVENT_EP_DATA_READY);
    if (endp_istat & USB_INTEP_DEV_OUT_2)
        usbd_event(USBD_EP2, USB_DEVICE_EVENT_EP_DATA_READY);
    if (endp_istat & USB_INTEP_DEV_IN_1)
        usbd_event(USBD_EP1, USB_DEVICE_EVENT_EP_WRITE_ACK);
    if (endp_istat & USB_INTEP_DEV_IN_2)
        usbd_event(USBD_EP2, USB_DEVICE_EVENT_EP_WRITE_ACK);

    ctrl_istat = endp_istat = 0;
    MAP_IntEnable(INT_USB0);
    return 0;
}

void ti_arm_mcu_usb_isr(void)
{
    ctrl_istat |= MAP_USBIntStatusControl(USB0_BASE);
    endp_istat |= MAP_USBIntStatusEndpoint(USB0_BASE);
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
    MAP_USBIntEnableEndpoint(USB0_BASE, USB_INTEP_0 | USB_INTEP_DEV_OUT_1 |
        USB_INTEP_DEV_OUT_2);

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
