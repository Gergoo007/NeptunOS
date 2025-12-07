#pragma once

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
};

pstruct usb_request {
	u8 bmRequestType;
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
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;
	u8 iManufacturer;
	u8 iProduct;
	u8 iSerialNumber;
	u8 bNumConfigurations;
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
