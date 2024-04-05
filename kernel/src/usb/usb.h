#pragma once

#include <usb/types.h>

typedef struct usb_dev {
	struct usb_dev* parent;
	void* ctrller;
	
	u8 port;
	u8 speed;
	u8 addr;
	u8 max_packet_size;
} usb_dev_t;

// USB Transaction
typedef struct usb_ta {
	usb_dev_req_t* req;
	void* data;
	u32 len;
	u8 complete;
	u8 success;
} usb_ta_t;

#include <usb/uhci.h>
