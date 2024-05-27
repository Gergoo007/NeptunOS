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

void pci_add_device(pci_hdr_t* dev) {
	if (dev->class == 0xc && dev->subclass == 0x3) {
		switch (dev->prog_if) {
			case 0: {
				printk("UHCI USB controller\n");
				uhci_init_controller(dev);
				break;
			}

			case 0x10: {
				printk("OHCI USB controller\n");
				ohci_init_controller(dev);
				break;
			}

			case 0x20: {
				printk("EHCI USB controller\n");
				break;
			}

			case 0x30: {
				printk("XHCI USB controller\n");
				break;
			}

			case 0x80: {
				// printk("PCI unknown USB controller\n");
				break;
			}
		}
	} else {
		// printk("PCI %s : %s %04x\n", pci_class(dev->class), pci_vendor(dev->vendor), dev->product);
	}
}
