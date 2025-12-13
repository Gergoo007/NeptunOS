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
	device_t* parent = nullptr;
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
			// This is always the host controller; if the device is on a hub then
			// the parent device points to that, otherwise (parent = hci)
			device_t* hci;
			const char* manufacturerName;
			const char* productName;
			const char* serial;
			u16 mps;
			u16 langid; // -1 if no STRING descriptors
			u8 addr;
			u8 portnum;
			punion {
				pstruct {
					u8 class_;
					u8 subclass;
					u8 progif;
				};
				u32 classcode : 24;
			};
			UsbSpeed speed;
		} USB;
	};
};

device_t& devmgr_add_device(device_t&& d);
