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
#include "usb/usbd_core.h"
#include "platform/platform.h"
#include "util/tp_types.h"
#include "util/tp_misc.h"
#include "util/debug.h"

#include "usb/usb_descs.c"

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

typedef struct {
    u8 bmRequestType;
    u8 bRequest;
    u16 wValue;
    u16 wIndex;
    u16 wLength;
} usb_setup_t;

typedef enum {
    USBD_STATE_DEFAULT = 0,
    USBD_STATE_ADDR_PENDING = 1,
    USBD_STATE_ADDR = 2,
} usbd_state_t;

#define EP0_SIZE 64

static u8 ep0_data[EP0_SIZE];
static usbd_state_t g_state;
static u16 g_addr;

static u8 *g_data;
static u16 g_remaining;

typedef void (*usb_req_handler_t)(usb_setup_t *setup);

static void usb_req_def_handler(usb_setup_t *setup)
{
    tp_out(("No handler for bRequest %d\n", setup->bRequest));
}

static int ep0_send(u8 *data, int len)
{
    if (len < EP0_SIZE)
    {
        g_remaining = 0;
        g_data = NULL;
        return platform.usb.ep0_data_send(data, len, 1);
    }

    g_remaining = len - EP0_SIZE;
    g_data = data + EP0_SIZE;
    return platform.usb.ep0_data_send(data, EP0_SIZE, 0);
}

static void set_configuration_handler(usb_setup_t *setup)
{
    tp_out(("SET_CONFIGURATION\n"));
    /* Just ack for now */
    platform.usb.ep0_data_ack(0);
    ep0_send(NULL, 0); /* Status ack */
}

#define MIN(a, b) ((a) > (b) ? (b) : (a))

static void get_descriptor_handler(usb_setup_t *setup)
{
    u8 id;
    u16 len;

    if (setup->bmRequestType != 0x80)
        return;

    platform.usb.ep0_data_ack(1);

    id = setup->wValue >> 8;
    tp_out(("GET_DESCRIPTOR: %d\n", id));
    switch (id)
    {
    case USB_DESC_DEVICE:
        len = MIN(setup->wLength, sizeof(usb_device_desc));
        ep0_send((u8 *)&usb_device_desc, len);
        break;
    case USB_DESC_CONFIGURATION:
        /* XXX: validate index, stall if necessary */
        ep0_send((u8 *)&usb_full_cfg_desc, sizeof(usb_full_cfg_desc));
        break;
    case USB_DESC_STRING:
        {
            int idx = setup->wValue & 0xff;

            tp_out(("index: %d\n", idx));
            len = usb_string_descs[idx][0];
            ep0_send((u8 *)usb_string_descs[idx], len);
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
    ep0_send(NULL, 0); /* Status ack */
    g_state = USBD_STATE_ADDR_PENDING;
}

static const usb_req_handler_t handlers[] = {
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

static void handle_ep0_data(void)
{
    int len;
    usb_setup_t *setup;

    len = platform.usb.ep0_data_get(ep0_data, sizeof(ep0_data));
    if (len < 0)
    {
        tp_err(("Failed to fetch EP0 data\n"));
        return;
    }

    setup = (usb_setup_t *)ep0_data;
#define P(field) //tp_out(("%s = %x\n", #field, setup->field))
    P(bmRequestType);
    P(bRequest);
    P(wValue);
    P(wIndex);
    P(wLength);

    if (setup->bRequest > ARRAY_SIZE(handlers))
    {
        /* XXX: stall */
        tp_err(("Malformed setup packet\n"));
        return;
    }

    handlers[setup->bRequest](setup);
}

void usbd_event(usbd_event_t event)
{
    tp_out(("%s: event %d\n", __FUNCTION__, event));
    switch (event)
    {
    case USB_DEVICE_EVENT_RESET:
    case USB_DEVICE_EVENT_EP0_WRITE_ACK:
        if (g_state == USBD_STATE_ADDR_PENDING)
        {
            platform.usb.set_addr(g_addr);
            g_state = USBD_STATE_ADDR;
        }
        if (g_remaining)
            ep0_send(g_data, g_remaining);
        return;
    case USB_DEVICE_EVENT_EP0_DATA_READY:
        handle_ep0_data();
        break;
    }
}
