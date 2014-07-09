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
#define USB_DESC_DEVICE 1
#define USB_DESC_CONFIGURATION 2
#define USB_DESC_STRING 3
#define USB_DESC_INTERFACE 4
#define USB_DESC_ENDPOINT 5
#define USB_DESC_DEVICE_QUALIFIER 6
#define USB_DESC_OTHER_SPEED_CONFIGURATION 7
#define USB_DESC_INTERFACE_POWER1 8

#define __packed __attribute__((packed))

typedef struct __packed {
    u8 bLength;
    u8 bDescriptorType;
    u16 bcdUSB;
    u8 bDeviceClass;
    u8 bDeviceSubClass;
    u8 bDeviceProtocol;
    u8 bMaxPacketSize0;
    u16 idVendor;
    u16 idProduct;
    u16 bcdDevice;
    u8 iManufacturer;
    u8 iProduct;
    u8 iSerialNumber;
    u8 bNumConfigurations;
} usb_device_desc_t;

typedef struct __packed {
    u8 bLength;
    u8 bDescriptorType;
    u16 wTotalLength;
    u8 bNumInterfaces;
    u8 bConfigurationValue;
    u8 iConfiguration;
    u8 bmAttributes;
    u8 bMaxPower;
} usb_cfg_desc_t;

typedef struct __packed {
    u8 bLength;
    u8 bDescriptorType;
    u8 bInterfaceNumber;
    u8 bAlternateSetting;
    u8 bNumEndpoints;
    u8 bInterfaceClass;
    u8 bInterfaceSubClass;
    u8 bInterfaceProtocol;
    u8 iInterface;
} usb_ifc_desc_t;

typedef struct __packed {
    u8 bLength;
    u8 bDescriptorType;
    u8 bEndpointAddress;
    u8 bmAttributes;
    u16 wMaxPacketSize;
    u8 bInterval;
} usb_endp_desc_t;

#define CDC_ACM_CS_INTERFACE 0x24

typedef struct __packed {
    u8 bFunctionLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u16 bcdCDC;
} usb_cdc_acm_header_func_desc_t;

typedef struct __packed {
    u8 bFunctionLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bmCapabilities;
} usb_cdc_acm_func_desc_t;

typedef struct __packed {
    u8 bFunctionLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bMasterInterface;
    u8 bSlaveInterface0;
} usb_cdc_acm_union_func_desc_t;

typedef struct __packed {
    u8 bFunctionLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bmCapabilities;
    u8 bDataInterface;
} usb_cdc_acm_call_mgmt_func_desc_t;

static const usb_device_desc_t usb_device_desc = {
    .bLength = sizeof(usb_device_desc),
    .bDescriptorType = USB_DESC_DEVICE,
    .bcdUSB = 0x110,
    .bDeviceClass = 2,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0xfefe,
    .idProduct = 0xdede,
    .bcdDevice = 0x100,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

/* Hard-coded CDC ACM for now */
static const struct __packed {
    usb_cfg_desc_t cfg;
    usb_ifc_desc_t ifc0;
    usb_cdc_acm_header_func_desc_t cdc_header;
    usb_cdc_acm_func_desc_t cdc_acm;
    usb_cdc_acm_union_func_desc_t cdc_union;
    usb_cdc_acm_call_mgmt_func_desc_t cdc_call_mgmt;
    usb_endp_desc_t ep1_in;
    usb_ifc_desc_t ifc1;
    usb_endp_desc_t ep2_in;
    usb_endp_desc_t ep1_out;
} usb_full_cfg_desc = {
    .cfg = {
        .bLength = sizeof(usb_cfg_desc_t),
        .bDescriptorType = USB_DESC_CONFIGURATION,
        .wTotalLength = sizeof(usb_full_cfg_desc),
        .bNumInterfaces = 2,
        .bConfigurationValue = 1,
        .iConfiguration = 4,
        .bmAttributes = (1<<6), /* Self powered */
        .bMaxPower = 250
    },
    .ifc0 = { 
        .bLength = sizeof(usb_ifc_desc_t),
        .bDescriptorType = USB_DESC_INTERFACE,
        .bInterfaceNumber = 0,
        .bAlternateSetting = 0,
        .bNumEndpoints = 1,
        .bInterfaceClass = 2,
        .bInterfaceSubClass = 2,
        .bInterfaceProtocol = 1,
        .iInterface = 0
    },
    .cdc_header = {
        .bFunctionLength = sizeof(usb_cdc_acm_header_func_desc_t),
        .bDescriptorType = CDC_ACM_CS_INTERFACE,
        .bDescriptorSubtype = 0x00,
        .bcdCDC = 0x0110,
    },
    .cdc_acm = {
        .bFunctionLength = sizeof(usb_cdc_acm_func_desc_t),
        .bDescriptorType = CDC_ACM_CS_INTERFACE,
        .bDescriptorSubtype = 0x02,
        .bmCapabilities = 0x06,
    },
    .cdc_union = {
        .bFunctionLength = sizeof(usb_cdc_acm_union_func_desc_t),
        .bDescriptorType = CDC_ACM_CS_INTERFACE,
        .bDescriptorSubtype = 0x06,
        .bMasterInterface = 0,
        .bSlaveInterface0 = 1,
    },
    .cdc_call_mgmt = {
        .bFunctionLength = sizeof(usb_cdc_acm_call_mgmt_func_desc_t),
        .bDescriptorType = CDC_ACM_CS_INTERFACE,
        .bDescriptorSubtype = 0x01,
        .bmCapabilities = 0x01,
        .bDataInterface = 1,
    },
    .ep1_in = {
        .bLength = sizeof(usb_endp_desc_t),
        .bDescriptorType = USB_DESC_ENDPOINT,
        .bEndpointAddress = 0x81,
        .bmAttributes = 0x3,
        .wMaxPacketSize = 0x0010,
        .bInterval = 1
    },
    .ifc1 = {
        .bLength = sizeof(usb_ifc_desc_t),
        .bDescriptorType = USB_DESC_INTERFACE,
        .bInterfaceNumber = 1,
        .bAlternateSetting = 0,
        .bNumEndpoints = 2,
        .bInterfaceClass = 10,
        .bInterfaceSubClass = 0,
        .bInterfaceProtocol = 0,
        .iInterface = 0
    },
    .ep2_in = {
        .bLength = sizeof(usb_endp_desc_t),
        .bDescriptorType = USB_DESC_ENDPOINT,
        .bEndpointAddress = 0x82,
        .bmAttributes = 0x2,
        .wMaxPacketSize = 0x0040,
        .bInterval = 0
    },
    .ep1_out = {
        .bLength = sizeof(usb_endp_desc_t),
        .bDescriptorType = USB_DESC_ENDPOINT,
        .bEndpointAddress = 0x01,
        .bmAttributes = 0x2,
        .wMaxPacketSize = 0x0040,
        .bInterval = 0
    },
};

#define USB_U16(u16) (u16) & 0xff, (u16) >> 8
#define _(c) c, 0
static const u8 *usb_string_descs[] = {
    [0] = (u8 []){ 4, USB_DESC_STRING, USB_U16(0x0409) },
#define STR_DESC(args...) (u8 []){ PP_NARG(args) + 2, USB_DESC_STRING, args }
    STR_DESC(_('d'),_('e'),_('v')),
    STR_DESC(_('T'),_('i'),_('n'),_('k'),_('e'),_('r'),_('P'),_('a'),_('l')),
    STR_DESC(_('1'),_('2'),_('3'),_('4'),_('5')),
    STR_DESC(_('c'),_('f'),_('g')),
};
#undef _
