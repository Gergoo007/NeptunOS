#pragma once

#include <util/storage.hh>
#include <util/smartptrs.hh>
#include <util/variant.hh>

#include <devmgr/msd/msd.hh>

enum struct DevmgrSubsys : i32 {
	NONE = -1,
	PCI,
	USB,
	MSD,
};

struct Device;
extern Vector<unique_ptr<Device>> devices;
struct usb_hci_interface_t;

enum struct UsbSpeed : u32 { LS, FS, HS, SS };

struct device_t_PCI {
	u16 vendor;
	u16 product;
	u8 class_;
	u8 subclass;
	u8 progif;
	u8 bus;
	u8 dev;
	u8 fun;
};

struct device_t_USB {
	u16 vendor;
	u16 product;
	// This is always the host controller; if the device is on a hub then
	// the parent device points to that, otherwise (parent = hci)
	Device* hci;
	String manufacturerName;
	String productName;
	String serial;
	u16 mps;
	u16 langid; // -1 if no STRING descriptors
	u8 addr;
	punion {
		pstruct {
			u8 class_;
			u8 subclass;
			u8 progif;
		};
		u32 classcode : 24;
	};
	UsbSpeed speed;
};

// Mass Storage Device
struct device_t_MSD {
	String manufacturerName;
	String productName;
	String serial;
	u64 size; // in bytes
	Vector<Partition> parts;
};

using devunion_t = variant<device_t_PCI, device_t_USB, device_t_MSD>;

// A bus-ok is eszköznek számítanak
struct Device {
	Device* parent = nullptr;
	void* extra;
	DevmgrSubsys subsys;
	u32 loc;

	devunion_t kinds;
};

Device& devmgr_add_device(Device&& d);
void devmgr_try_mount(filesystem& f);
