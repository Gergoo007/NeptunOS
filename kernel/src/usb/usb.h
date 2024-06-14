#pragma once

#include <usb/types.h>
#include <lib/bitmap.h>

enum {
	HC_UHCI = 0,
	HC_OHCI = 1,
	HC_EHCI = 2,
	HC_XHCI = 3,
};

typedef struct usb_dev {
	void* hc;
	char* product;
	char* manufacturer;
	u8 hc_type;
	u8 port;

	u8 addr : 7;
	u8 ls : 1;
	u8 endp : 4;
} usb_dev;

// USB Transaction
typedef struct usb_ta {
	usb_request* req;
	void* data;
	u32 len;
	u8 complete;
	u8 success;
} usb_ta;

extern usb_dev* usb_devs;
extern u32 num_usb_devs;

extern bitmap addr_bm;

#include <usb/hci/uhci.h>
