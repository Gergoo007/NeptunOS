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

enum struct UsbSpeed { LS, FS, HS, SS };

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
			device_t* hci;
			const char* manufacturerName;
			const char* productName;
			const char* serial;
			u16 mps;
			u16 langid; // -1 if no STRING descriptors
			u8 addr;
			u8 hci_portnum;
			u8 class_;
			u8 subclass;
			u8 progif;
			UsbSpeed speed;
		} USB;
	};
};

device_t& devmgr_add_device(device_t&& d);
