#include "ahci.h"
#include <neptunos/graphics/text_renderer.h>

hba_mem_regs_t* abar = NULL;

void ahci_init(pci_device_header_0_t* device) {
	reportln("Started AHCI initialization...");
	reportln("Device to be used for AHCI: %04x:%04x > %s %s", device->base.vendor_id, device->base.device_id, 
		pci_find_class(device->base.class), 
		pci_find_subclass(device->base.class, device->base.subclass));

	abar = (hba_mem_regs_t*)((uint64_t)device->base_addr_5);
	map_page(abar, abar, MAP_FLAGS_IO_DEFAULTS);

	assert(offsetof(hba_mem_regs_t, ports) != 0x0100, "Port registers misplaced in hba_mem_regs_t");
	
}
