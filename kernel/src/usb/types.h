#pragma once

#include <lib/types.h>

#pragma pack(1)

enum {
	USB_PID_OUT 	= 0b11100001,
	USB_PID_IN 		= 0b01101001,
	USB_PID_SOF 	= 0b10100101,
	USB_PID_SETUP 	= 0b00101101,
	USB_PID_DATA0 	= 0b11000011,
	USB_PID_DATA1 	= 0b01001011,
	USB_PID_DATA2 	= 0b10000111,
	USB_PID_MDATA 	= 0b00001111,
	USB_PID_ACK 	= 0b11010010,
	USB_PID_NAK 	= 0b01011010,
	USB_PID_STALL 	= 0b00011110,
	USB_PID_NYET 	= 0b10010110,
	USB_PID_PRE 	= 0b00111100,
	USB_PID_ERR 	= 0b00111100,
	USB_PID_SPLIT 	= 0b01111000,
	USB_PID_PING 	= 0b10110100,
};

enum {
	USB_HS_ACK,
	USB_HS_NAK,
	USB_HS_STALL,
	USB_HS_NYET,
	USB_HS_ERR,
};

enum {
	USB_DEV_ATTACHED,
	USB_DEV_POWERED,
	USB_DEV_DEFAULT_STATE,
	USB_DEV_ADDRESS,
	USB_DEV_CONFIGURED,
	USB_DEV_SUSPENDED,
};

enum {
	RT_TRANSFER_MASK 	= 0x80,
	RT_DEV_TO_HOST		= 0x80,
	RT_HOST_TO_DEV		= 0x00,

	RT_TYPE_MASK		= 0x60,
	RT_STANDARD			= 0x00,
	RT_CLASS			= 0x20,
	RT_VENDOR			= 0x40,

	RT_RECIPIENT_MASK	= 0x1f,
	RT_DEV				= 0x00,
	RT_INTF				= 0x01,
	RT_ENDP				= 0x02,
	RT_OTHER			= 0x03,
};

enum {
	USB_DESC_DEVICE = 1,
	USB_DESC_CONFIGURATION = 2,
	USB_DESC_STRING = 3,
	USB_DESC_INTERFACE = 4,
	USB_DESC_ENDPOINT = 5,
	USB_DESC_DEVICE_QUALIFIER = 6,
	USB_DESC_OTHER_SPEED_CONFIGURATION = 7,
	USB_DESC_INTERFACE_POWER = 8,
};

typedef struct usb_request {
	u8 req_type;
	u8 req;
	u16 value;
	u16 index;
	u16 length;
} usb_request_t;

typedef struct usb_desc_dev {
	u8 len;
	u8 desc_type; // DEVICE
	u16 usb_ver; // in BCD! pl. 0x210 = USB 2.10
	u8 class;
	u8 subclass;
	u8 protocol;
	u8 max_packet_size; // for endpoint zero
	u16 vendor;
	u16 product;
	u16 dev_rev; // in BCD!
	u8 manufacturer_index; // Index in STRING descriptor
	u8 product_index; // Index in STRING descriptor
	u8 serial_num_index; // Index in STRING descriptor
	u8 num_configs;
} usb_desc_dev_t;

typedef struct usb_desc_dev_qualifier {
	u8 len;
	u8 desc_type; // DEVICE_QUALIFIER
	u16 usb_ver; // in BCD! pl. 0x210 = USB 2.10
	u8 class;
	u8 subclass;
	u8 protocol;
	u8 max_packet_size;
	u8 num_configs;
	u8 : 8; // reserved, must be zero
} usb_desc_dev_qualifier_t;

typedef struct usb_desc_configuration {
	u8 len;
	u8 desc_type; // CONFIGURATION

	// Length of everything returned by the request for CONFIGURATION summed
	// (CONFIGURATION, INTERFACE, ENDPOINT, class- and vendor-specific)
	u16 len_total;

	u8 num_interfaces;
	u8 config;
	u8 config_index; // Index in STRING descriptor
	u8 attributes; // bitmap, TODO: break up into separate struct
	u8 max_power; // in 2 mA's
} usb_desc_configuration_t;

typedef struct usb_desc_interface {
	u8 len;
	u8 desc_type; // INTERFACE
	u8 interface_num;
	u8 alternate_setting;
	u8 num_endpoints;
	u8 class;
	u8 subclass;
	u8 protocol;
	u8 interface_index; // Index in STRING descriptor
} usb_desc_interface_t;

typedef struct usb_desc_endpoint {
	u8 len;
	u8 desc_type; // ENDPOINT
	u8 endpoint_addr; // bitmap, TODO: break up into separate struct
	u8 attributes; // bitmap, TODO: break up into separate struct
	u16 max_packet_size; // bitmap, TODO: break up into separate struct
	u8 interval; // Polling interval, in microframes or frames
} usb_desc_endpoint_t;

typedef struct usb_desc_string {
	u8 len;
	u8 desc_type; // STRING
	wchar string[0]; // Unicode-encoded, non-null terminated string
} usb_desc_string_t;
