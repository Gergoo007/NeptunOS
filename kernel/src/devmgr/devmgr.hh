#pragma once

#include <util/storage.hh>

namespace devmgr {
	enum struct Bus {
		PCI,
		USB,
		CHIPSET
	};

	struct Device {
		struct PCIProperties {
			u16 vendor, product;
			u8 class_, subclass, progif;
			u8 bus, dev, fun;
		};

		String vendor;
		String product;
		Variant<PCIProperties> properties;
		Opt<String> acpiObject;

		Device(String v, String p): vendor(move<String>(v)), product(move<String>(p)) {  };
	};

	extern Vector<Device> devs;
}
