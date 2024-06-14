#include <acpi/mcfg.h>

void mcfg_parse(mcfg* _mcfg) {
	u32 entries = (_mcfg->base.len - sizeof(mcfg)) / sizeof(mcfg_entry);

	// Enumerate the PCIe bus
	foreach (i, entries) {
		for (u32 bus = _mcfg->entries[i].pci_bus_start; bus < _mcfg->entries[i].pci_bus_end; bus++) {
			void* bus_address = (void*) (_mcfg->entries[i].base_address + (bus << 20));

			foreach (device, 32) {
				void* dev_addr = (void*) (bus_address + (device << 15));
				pci_hdr* dev = dev_addr;

				if (dev->vendor == 0xffff) continue;

				foreach (f, 8) {
					pci_hdr* func = (pci_hdr*) (dev_addr + (f << 12));
					
					if (func->vendor == 0 || func->vendor == 0xffff) continue;

					pci_add_device(func);
				}
			}
		}
	}
}
