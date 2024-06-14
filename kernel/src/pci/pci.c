#include <pci/pci.h>

char str[5];

char* pci_vendor(u16 id) {
	for (u32 i = 0; i < sizeof(vendors) / 10; i++) {
		if (vendors[i].id == id)
			return vendors[i].name;
	}

	hexn_to_str(id, str, 4);
	return str;
}

char* pci_class(u8 class) {
	if (class > 0x14) {
		hexn_to_str(class, str, 2);
		return str;
	}

	return classes[class];
}

void pci_add_device(pci_hdr* dev) {
	if (dev->class == 0xc && dev->subclass == 0x3) {
		switch (dev->prog_if) {
			case 0: {
				report("UHCI USB controller");
				uhci_init_controller(dev);
				break;
			}

			case 0x10: {
				report("OHCI USB controller");
				ohci_init_controller(dev);
				break;
			}

			case 0x20: {
				report("EHCI USB controller");
				break;
			}

			case 0x30: {
				report("XHCI USB controller");
				break;
			}

			case 0x80: {
				// printk("PCI unknown USB controller\n");
				break;
			}
		}
	} else if (dev->class == 0x01 && dev->subclass == 0x06) {
		ahci_init(dev);
	}else {
		// printk("PCI %s : %s %04x\n", pci_class(dev->class), pci_vendor(dev->vendor), dev->product);
	}
}
