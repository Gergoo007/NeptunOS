#include "pci.h"
#include "pci_ids.h"

void pci_enumerate() {
	uint64_t num_entries = (mcfg->hdr.length - sizeof(mcfg_hdr_t)) / sizeof(mcfg_entry_t);

	for (uint32_t i = 0; i < num_entries; i++) {
		mcfg_entry_t* entry = (mcfg_entry_t*)((uint64_t)mcfg + sizeof(mcfg_hdr_t) + (sizeof(mcfg_entry_t) * i));
		map_address((void*)entry->base_addr, (void*)entry->base_addr);

		for (uint64_t i = entry->bus_start; i < entry->bus_end; i++) {
			for (uint8_t j = 0; j < 32; j++) {
				for (uint8_t k = 0; k < 8; k++) {
					// Credit for this formula: https://wiki.osdev.org/PCI_Express
					pci_device_header_t* device = (pci_device_header_t*)(entry->base_addr + ((i - 0) << 20 | j << 15 | k << 12));
					map_address((void*)device, (void*)device);
					if (device->vendor_id != 0x0000 && device->vendor_id != 0xffff) {
						printk("Detected device: %s %s: %s > %s\n", pci_find_vendor(device->vendor_id), 
							pci_find_product(device->device_id), pci_find_class(device->class), 
							pci_find_subclass(device->class, device->subclass));
					}
				}
			}
		}
	}
}
