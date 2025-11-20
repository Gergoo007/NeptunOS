#pragma once

#include <util/storage.hh>

enum struct Subsystems : i32 {
	NONE = -1,
	PCI,
	USB,
};


// A bus-ok is eszköznek számítanak
struct device_t {
	Subsystems subsys;
	union {
		struct {
			u16 vendor, product;
			u8 class_, subclass, progif;
			u8 bus, dev, fun;
		} PCI;

		struct {
			u16 vendor, product;
			u8 class_, subclass, progif;
			device_t* hci;
		} USB;
	};
};

extern vector<device_t> devices;

void devmgr_add_device(device_t&& d);
