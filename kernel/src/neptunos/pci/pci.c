#include "pci.h"

#include <neptunos/ahci/ahci.h>

void pci_enumerate(void) {
	uint64_t num_entries = (mcfg->hdr.length - sizeof(mcfg_hdr_t)) / sizeof(mcfg_entry_t);

	for (uint32_t i = 0; i < num_entries; i++) {
		mcfg_entry_t* entry = (mcfg_entry_t*)((uint64_t)mcfg + sizeof(mcfg_hdr_t) + (sizeof(mcfg_entry_t) * i));
		map_page((void*)entry->base_addr, (void*)entry->base_addr, MAP_FLAGS_IO_DEFAULTS);

		for (uint64_t i = entry->bus_start; i < entry->bus_end; i++) {
			for (uint8_t j = 0; j < 32; j++) {
				for (uint8_t k = 0; k < 8; k++) {
					// Credit for this formula: https://wiki.osdev.org/PCI_Express
					pci_device_header_0_t* device = (pci_device_header_0_t*)(entry->base_addr + ((i - 0) << 20 | j << 15 | k << 12));
					//pci_device_header_t* device = (pci_device_header_t*)(entry->base_addr + ((i - 0) << 20 | j << 15 | k << 12));
					map_page((void*)device, (void*)device, MAP_FLAGS_IO_DEFAULTS);
					if (device->base.vendor_id != 0x0000 && device->base.vendor_id != 0xffff) {
						reportln("Detected device: %s %s: %s > %s", pci_find_vendor(device->base.vendor_id),
							pci_find_product(device->base.device_id), pci_find_class(device->base.class),
							pci_find_subclass(device->base.class, device->base.subclass));

						if (device->base.class == PCI_CLASS_MSD && device->base.subclass == PCI_SCLASS_SATA
						&& device->base.program_interface == PCI_PI_AHCI_1_0) {
							ahci_init(device);
						}
					}
				}
			}
		}
	}
}
