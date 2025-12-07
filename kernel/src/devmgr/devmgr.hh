#pragma once

#include <util/storage.hh>
#include <util/smartptrs.hh>

enum struct DevmgrSubsys : i32 {
	NONE = -1,
	PCI,
	USB,
};

struct device_t;
extern vector<unique_ptr<device_t>> devices;
struct usb_hci_interface_t;

// A bus-ok is eszköznek számítanak
struct device_t {
	DevmgrSubsys subsys;
	union {
		struct {
			u16 vendor;
			u16 product;
			void* extra;
			u8 class_;
			u8 subclass;
			u8 progif;
			u8 bus;
			u8 dev;
			u8 fun;
		} PCI;

		struct {
			u16 vendor;
			u16 product;
			// usb_hci_interface_t* hci;
			device_t* hci;
			u8 hci_portnum;
			u16 mps;
			u8 class_;
			u8 subclass;
			u8 progif;
			bool ls;
		} USB;
	};
};

device_t& devmgr_add_device(device_t&& d);
