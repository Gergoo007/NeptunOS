#pragma once

#include "devmgr/devmgr.hh"
#include <types.hh>

struct UsbPid {
	static constexpr u8 SETUP	= 0b00101101;
	static constexpr u8 IN		= 0b01101001;
	static constexpr u8 OUT		= 0b11100001;
};

struct UsbRequests {
	static constexpr u8 GET_STATUS			=	0;
	static constexpr u8 CLEAR_FEATURE		=	1;
	static constexpr u8 SET_FEATURE			=	3;
	static constexpr u8 SET_ADDRESS			=	5;
	static constexpr u8 GET_DESCRIPTOR		=	6;
	static constexpr u8 SET_DESCRIPTOR		=	7;
	static constexpr u8 GET_CONFIGURATION	=	8;
	static constexpr u8 SET_CONFIGURATION	=	9;
	static constexpr u8 GET_INTERFACE		=	10;
	static constexpr u8 SET_INTERFACE		=	11;
	static constexpr u8 SYNC_FRAME			=	12;

	// HID
	static constexpr u32 GET_REPORT		= 0x01;
	static constexpr u32 GET_IDLE		= 0x02;
	static constexpr u32 GET_PROTOCOL	= 0x03;
	static constexpr u32 SET_REPORT		= 0x09;
	static constexpr u32 SET_IDLE		= 0x0A;
	static constexpr u32 SET_PROTOCOL	= 0x0B;
};

struct UsbDescriptors {
	static constexpr u8 DEVICE				=	0x01;
	static constexpr u8 CONFIGURATION		=	0x02;
	static constexpr u8 STRING				=	0x03;
	static constexpr u8 INTERFACE			=	0x04;
	static constexpr u8 ENDPOINT			=	0x05;

	// HID
	static constexpr u8 HID					=	0x21;
	static constexpr u8 REPORT				=	0x22;
};

struct UsbClass {
	static constexpr u8 HID		= 0x03;
	static constexpr u8 HUB		= 0x09;
};

pstruct usb_request {
	punion {
		u8 bmRequestType;
		pstruct {
			u8 d2h : 1;

			// 0: standard
			// 1: class
			// 2: vendor
			// 3: reserved
			u8 type : 2;

			// 0: device
			// 1: interface
			// 2: endpoint
			// 3: other
			u8 recipient : 5;
		};
	};
	u8 bRequest;
	punion {
		u16 wValue;
		pstruct {
			u8 wValueL;
			u8 wValueH;
		};
	};
	u16 wIndex;
	u16 wLength;
};

pstruct usb_descriptor_header {
	u8 bLength;
	u8 bDescriptorType;
};

pstruct usb_descriptor_device {
	usb_descriptor_header hdr;
	u16 bcdUSB;
	punion {
		pstruct {
			u8 bDeviceClass;
			u8 bDeviceSubClass;
			u8 bDeviceProtocol;
		};
		u32 classcode : 24;
	};
	u8 bMaxPacketSize;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;
	u8 iManufacturer;
	u8 iProduct;
	u8 iSerialNumber;
	u8 bNumConfigurations;
};

static_assert(offsetof(usb_descriptor_device, bMaxPacketSize) == 7);

pstruct usb_descriptor_configuration {
	usb_descriptor_header hdr;
	u16 wTotalLength;
	u8 bNumInterfaces;
	u8 bConfigurationValue;
	u8 iConfiguration;
	pstruct {
		u8 : 5;
		u8 remote_wakeup : 1;
		u8 self_powered : 1;
		u8 : 1;
	} bmAttributes;
	u8 bMaxPower;
};

pstruct usb_descriptor_interface {
	usb_descriptor_header hdr;
	u8 bInterfaceNumber;
	u8 bAlternateSetting;
	u8 bNumEndpoints;
	u8 bInterfaceClass;
	u8 bInterfaceSubClass;
	u8 bInterfaceProtocol;
	u8 iInterface;
};

pstruct usb_descriptor_endpoint {
	usb_descriptor_header hdr;
	punion {
		pstruct {
			u8 endp_num : 4;
			u8 : 3;
			u8 endp_dir : 1; // 1 => IN, 0 => OUT
		};
		u8 bEndpointAddress;
	};
	punion {
		pstruct {
			// 0 => ctl; 1 => iso; 2 => bulk; 3 => int
			u8 attr_transfer_type : 2;
			u8 attr_sync_type : 2;
			u8 attr_usage_type : 2;
			u8 : 2;
		};
		u8 bmAttributes;
	};
	punion {
		pstruct {
			u16 mps : 11;
			// Only valid for HS; TODO: might wanna set this in EHCI driver
			u16 additional_ta_per_uframe : 2;
			u16 : 3;
		};
		u16 wMaxPacketSize;
	};

	// For HS devices, this is the number of uframes (125 us), for LS/FS frames (1ms)
	// FS/HS iso, HS int: 2^(bInterval - 1)
	// Everything else: number of (u)frames
	u8 bInterval;
};

pstruct usb_descriptor_hub {
	usb_descriptor_header hdr;
	u8 bNbrPorts;
	u8 wHubCharacteristics;
	u8 bPowerOnGood;
	u8 bHubContrCurrent;
	u8 bmRemovable[];
};

punion usb_hub_port_feats {
	pstruct {
		u16 PORT_CONNECTION : 1;
		u16 PORT_ENABLE : 1;
		u16 PORT_SUSPEND : 1;
		u16 PORT_OVER_CURRENT : 1;
		u16 PORT_RESET : 1;
		u16 : 3;
		u16 PORT_POWER : 1;
		u16 PORT_LOW_SPEED : 1;
		u16 PORT_HIGH_SPEED : 1;
		u16 : 5;
		u16 C_PORT_CONNECTION : 1;
		u16 C_PORT_ENABLE : 1;
		u16 C_PORT_SUSPEND : 1;
		u16 C_PORT_OVER_CURRENT : 1;
		u16 C_PORT_RESET : 1;
		u16 PORT_TEST : 1;
		u16 PORT_INDICATOR : 1;
	};
	u32 raw : 24;

	usb_hub_port_feats(u32 r): raw(r) {  }
};

pstruct usb_descriptor_string_langids {
	usb_descriptor_header hdr;
	u16 wLangID[];
};

pstruct usb_descriptor_string {
	usb_descriptor_header hdr;
	wchar string[];
};

struct device_t;
struct usb_hci_interface_t {
	void (*usb_send)(device_t& usbdev, u8 endp, usb_request* request, void* databuf);
	void (*usb_reset_port)(device_t& usbdev);
	u8 (*usb_make_address)(device_t& hcidev);
};

void usb_init_all();
void usb_init(device_t& usbdev);
device_t& usb_device_add_skeleton(device_t& parent, u8 port, UsbSpeed speed);
