#include <pci/strings.h>

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
	printk("PCI %s : %s %04x\n", pci_class(dev->class), pci_vendor(dev->vendor), dev->product);
}
