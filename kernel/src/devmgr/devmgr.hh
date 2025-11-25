#pragma once

#include <util/storage.hh>

enum struct DevmgrSubsys : i32 {
	NONE = -1,
	PCI,
	USB,
};


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
			u16 mps;
			u8 class_;
			u8 subclass;
			u8 progif;
		} USB;
	};
};

extern vector<device_t> devices;

device_t& devmgr_add_device(device_t&& d);
