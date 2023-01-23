#include "pci_ids.h"

const pci_id_pair pci_vendors[] = {
	{ 0x8086, "Intel" },
	{ 0x1234, "General emulator" }
};

const pci_id_pair pci_products[] = {
	{ 0x29c0, "82G33/G31/P35/P31 Express DRAM Controller" },
	{ 0x1111, "Virtual graphics" },
	{ 0x2918, "82801IB (ICH9) LPC Interface Controller" },
	{ 0x2922, "82801IR/IO/IH (ICH9R/DO/DH) 6 port SATA Controller [AHCI mode]" },
	{ 0x2930, "82801I (ICH9 Family) SMBus Controller" }
};

char* pci_find_vendor(uint16_t id) {
	for (uint16_t i = 0; i < sizeof(pci_vendors); i++)
		if (pci_vendors[i].id == id)
			return pci_vendors[i].name;
	
	printk("\nError: Unknown manufacturer: %04x\n", id);
	return NULL;
}

char* pci_find_product(uint16_t id) {
	for (uint16_t i = 0; i < sizeof(pci_products); i++)
		if (pci_products[i].id == id)
			return pci_products[i].name;
			
	printk("\nError: Unknown product: %04x\n", id);
	return NULL;
}
