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

typedef struct usb_dev_req {
	u8 req_type;
	u8 req;
	u16 value;
	u16 index;
	u16 data_length;
} usb_dev_req_t;

static usb_dev_req_t USB_SET_ADDRESS = {
	.req_type = 0,
	.req = 5,
	// .value is to be filled!
	.index = 0,
	.data_length = 0,
};

static usb_dev_req_t USB_GET_DESC = {
	.req_type = 0b10000000,
	.req = 6,
	.value = 1, /* (index << 8) | type */
	.index = 0,
	.data_length = 1, /* descriptor_length */
};

static usb_dev_req_t USB_GET_DESC_DEV = {
	.req_type = 0b10000000,
	.req = 6,
	.value = 0b0000000100000000,
	.index = 0,
	.data_length = 18,
};

static usb_dev_req_t USB_SET_DESC = {
	.req_type = 0b00000000,
	.req = 7,
	.value = 1, /* (index << 8) | type */
	.index = 0,
	.data_length = 1, /* descriptor_length */
};

static usb_dev_req_t USB_GET_CONFIG = {
	.req_type = 0b10000000,
	.req = 8,
	.value = 0,
	.index = 0,
	.data_length = 1,
};

static usb_dev_req_t USB_SET_CONFIG = {
	.req_type = 0b00000000,
	.req = 9,
	.value = 0, // config
	.index = 0,
	.data_length = 0,
};

static usb_dev_req_t USB_GET_INTERF = {
	.req_type = 0b10000001,
	.req = 10,
	.value = 0,
	.index = 0, // interface
	.data_length = 1,
};

static usb_dev_req_t USB_SET_INTERF = {
	.req_type = 0b00000001,
	.req = 11,
	.value = 0, // alternate setting
	.index = 0, // interface
	.data_length = 0,
};

static usb_dev_req_t USB_CLEAR_FEATURE = {
	.req_type = 0b0, // needs to be set to the appropiate value
	.req = 1,
	.value = 0, // feature selector
	.index = 0, // interface/endpoint/zero
	.data_length = 1,
};

static usb_dev_req_t USB_SET_FEATURE = {
	.req_type = 0b0, // needs to be set to the appropiate value
	.req = 3,
	.value = 0, // feature selector
	.index = 0, // (test selector) | (interface/endpoint/zero << 8)
	.data_length = 0,
};

static usb_dev_req_t USB_GET_STATUS = {
	.req_type = 0b10000000, // needs to be set to the appropiate value
	.req = 0,
	.value = 0,
	.index = 0, // interface/endpoint/zero
	.data_length = 2,
};

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
	u16 lang_ids[0]; // Supported languages
} usb_desc_string_t;

typedef struct usb_desc_string_response {
	u8 len;
	u8 desc_type; // STRING
	u16 lang_ids[0]; // Supported languages
	char string[0]; // Unicode-encoded, non-null terminated string
} usb_desc_string_response_t;

// Address (for selecting a function)
// 7 bits wide
// For IN, SETUP, OUT, PING, SPLIT

// Endpoint
// 4 bits wide
// For IN, SETUP, OUT, PING

// Data is sent least-significant bit first

// Transactions are made out of packets
// A Transaction starts with a token packet, then the data packet(s), then the ACK packet
// IN or OUT transactions are often referred to, collectively, as DATA transactions

// A USB device (function) consists of 1 or more configurations,
// which contain 1 or more interfaces,
// which in turn contain 0 or more endpoints

// USB Device Enumeration

// 1. 	The hub to which the device has been attached notifies the host via its status change pipe.
//		The newly attached device is in the powered state at this point, and the port to which it has been attached is disabled.
// 2. 	The host queries more information from the hub to determine that a device has been attached, and to which port.
// 3. 	The host must wait at least 100 milliseconds to allow a device to complete
//		its insertion process, and for power to stabilize at the device.
//		After the delay, the host enables the port and issues a reset signal to the device for at least 50 milliseconds.
// 4. 	The hub performs any required reset processing. After the reset signal has been released,
//		the port is enabled and the device enters the default state.
// 5. 	The host assigns the device a unique address, thereby transitioning the device into the address state.
// 6. 	The host requests the device descriptor from the device via the default control pipe
//		in order to determine the actual maximum data payload size of the default control pipe for the device.
//		This step may occur before or after the host assigns the device an address.
// 7. 	The host reads all the possible device configuration information.
// 8. 	The host selects a certain configuration from the list of configurations supported by the device
//		and sets the device to use that configuration.
//		Optionally, the host may also select alternate interface settings within a configuration.
//		All endpoints are initialized as described by the selected configuration, and the device is ready to use.
