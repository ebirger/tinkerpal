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
#include "usb/usbd_core_platform.h"
#include "usb/usbd_core.h"
#include "platform/platform.h"
#include "util/tp_misc.h"
#include "util/debug.h"

#ifdef CONFIG_USB_CDC_ACM
#include "usb/cdc_acm_descs.c"
#else
#error No USB device class defined
#endif

#define USB_REQ_GET_STATUS 0
#define USB_REQ_CLEAR_FEATURE 1
#define USB_REQ_SET_FEATURE 3
#define USB_REQ_SET_ADDRESS 5
#define USB_REQ_GET_DESCRIPTOR 6
#define USB_REQ_SET_DESCRIPTOR 7
#define USB_REQ_GET_CONFIGURATION 8
#define USB_REQ_SET_CONFIGURATION 9
#define USB_REQ_GET_INTERFACE 10
#define USB_REQ_SET_INTERFACE 11
#define USB_REQ_SYNCH_FRAME 12

#define USB_REQ_TYPE_STD 0
#define USB_REQ_TYPE_CLASS 1
#define USB_REQ_TYPE_VENDOR 2

typedef enum {
    USBD_STATE_DEFAULT = 0,
    USBD_STATE_ADDR_PENDING = 1,
    USBD_STATE_ADDR = 2,
} usbd_state_t;

#define EP0_SIZE 64

static void handle_setup(void);

static u8 ep0_data[EP0_SIZE];
static usbd_state_t g_state;
static u16 g_addr;

static u8 *g_send_data;
static u16 g_send_data_remaining;

static u8 *g_recv_data = ep0_data;
static u16 g_recv_data_remaining = sizeof(usb_setup_t);

static data_ready_cb_t g_data_ready_cb = handle_setup;

static void usb_req_def_handler(usb_setup_t *setup)
{
    tp_out(("No handler for bRequest %d\n", setup->bRequest));
}

int usbd_ep0_send(u8 *data, int len)
{
    if (len < EP0_SIZE)
    {
        g_send_data_remaining = 0;
        g_send_data = NULL;
        return platform.usb.ep0_data_send(data, len, 1);
    }

    g_send_data_remaining = len - EP0_SIZE;
    g_send_data = data + EP0_SIZE;
    return platform.usb.ep0_data_send(data, EP0_SIZE, 0);
}

void usbd_ep0_wait_for_data(u8 *data, int len, data_ready_cb_t cb)
{
    g_recv_data = data;
    g_recv_data_remaining = len;
    g_data_ready_cb = cb;
}

static void set_configuration_handler(usb_setup_t *setup)
{
    tp_out(("SET_CONFIGURATION\n"));
    /* Just ack for now */
    platform.usb.ep0_data_ack(0);
    usbd_ep0_send(NULL, 0); /* Status ack */
}

#define MIN(a, b) ((a) > (b) ? (b) : (a))

static void get_descriptor_handler(usb_setup_t *setup)
{
    u16 len;

    if (setup->bmRequestType != 0x80)
        return;

    platform.usb.ep0_data_ack(1);

    switch (setup->wValue >> 8)
    {
    case USB_DESC_DEVICE:
        tp_out(("GET_DESCRIPTOR: DEVICE\n"));
        len = MIN(setup->wLength, sizeof(usb_device_desc));
        usbd_ep0_send((u8 *)&usb_device_desc, len);
        break;
    case USB_DESC_CONFIGURATION:
        {
            usb_cfg_desc_t *cfg_header = (usb_cfg_desc_t *)&usb_full_cfg_desc;

            tp_out(("GET_DESCRIPTOR: CONFIGURATION\n"));
            /* XXX: validate index, stall if necessary */
            len = MIN(setup->wLength, cfg_header->wTotalLength);
            tp_out(("---------------------------------\n"));
            hexdump((u8 *)&usb_full_cfg_desc, len);
            tp_out(("---------------------------------\n"));
            usbd_ep0_send((u8 *)&usb_full_cfg_desc, len);
        }
        break;
    case USB_DESC_STRING:
        {
            const u8 *str = usb_string_descs[setup->wValue & 0xff];

            len = MIN(setup->wLength, str[0]);
            usbd_ep0_send((u8 *)str, len);
        }
        break;
    }
}

static void set_addr_handler(usb_setup_t *setup)
{
    if (setup->bmRequestType || setup->wIndex || setup->wLength)
        return;

    if (setup->wValue == 0)
    {
        g_state = USBD_STATE_DEFAULT;
        g_addr = 0;
        return;
    }

    g_addr = setup->wValue;
    tp_out(("Set Address: %d\n", g_addr));
    platform.usb.ep0_data_ack(0);
    usbd_ep0_send(NULL, 0); /* Status ack */
    /* USB spec mandates address can't be set before the end of the status
     * stage.
     */
    g_state = USBD_STATE_ADDR_PENDING;
}

static const usb_req_handler_t std_req_handlers[] = {
    [USB_REQ_GET_STATUS] = usb_req_def_handler,
    [USB_REQ_CLEAR_FEATURE] = usb_req_def_handler,
    [USB_REQ_SET_FEATURE] = usb_req_def_handler,
    [USB_REQ_SET_ADDRESS] = set_addr_handler,
    [USB_REQ_GET_DESCRIPTOR] = get_descriptor_handler,
    [USB_REQ_SET_DESCRIPTOR] = usb_req_def_handler,
    [USB_REQ_GET_CONFIGURATION] = usb_req_def_handler,
    [USB_REQ_SET_CONFIGURATION] = set_configuration_handler,
    [USB_REQ_GET_INTERFACE] = usb_req_def_handler,
    [USB_REQ_SET_INTERFACE] = usb_req_def_handler,
    [USB_REQ_SYNCH_FRAME] = usb_req_def_handler,
};

void usbd_dump_setup(usb_setup_t *setup)
{
#define P(field) tp_out(("%s = %x\n", #field, setup->field))
    P(bmRequestType);
    P(bRequest);
    P(wValue);
    P(wIndex);
    P(wLength);
}

static void handle_setup(void)
{
    usb_setup_t *setup;
    int req_type;

    setup = (usb_setup_t *)ep0_data;
    req_type = (setup->bmRequestType & ((1<<5)|(1<<6))) >> 5;

    switch (req_type)
    {
    case USB_REQ_TYPE_STD:
        if (setup->bRequest > ARRAY_SIZE(std_req_handlers))
            goto Error;

        std_req_handlers[setup->bRequest](setup);
        break;
    default:
        goto Error;
    }

    return;

Error:
    /* XXX: stall EP0 */
    tp_err(("Unsupported request:\n"));
    usbd_dump_setup(setup);
    return;
}

void usbd_event(usbd_event_t event)
{
    tp_out(("%s: event %d\n", __FUNCTION__, event));
    switch (event)
    {
    case USB_DEVICE_EVENT_RESET:
        return;
    case USB_DEVICE_EVENT_EP0_WRITE_ACK:
        if (g_state == USBD_STATE_ADDR_PENDING)
        {
            platform.usb.set_addr(g_addr);
            g_state = USBD_STATE_ADDR;
        }
        if (g_send_data_remaining)
            usbd_ep0_send(g_send_data, g_send_data_remaining);
        return;
    case USB_DEVICE_EVENT_EP0_DATA_READY:
        {
            int len;

            len = MIN(g_recv_data_remaining, EP0_SIZE);
            len = platform.usb.ep0_data_get(g_recv_data, len);
            if (len < 0)
            {
                /* XXX: stall, signal upper layer */
                tp_err(("Failed to fetch EP0 data\n"));
                return;
            }
            g_recv_data_remaining -= len;
            if (g_recv_data_remaining == 0)
            {
                data_ready_cb_t cb = g_data_ready_cb;

                /* Default waiting for setup packet */
                usbd_ep0_wait_for_data(ep0_data, sizeof(usb_setup_t),
                    handle_setup);
                cb();
            }
        }
        break;
    }
}
