#include <acpi/mcfg.h>

void mcfg_parse(mcfg_t* mcfg) {
	u32 entries = (mcfg->base.len - sizeof(mcfg_t)) / sizeof(mcfg_entry_t);

	// Enumerate the PCIe bus
	foreach (i, entries) {
		for (u32 bus = mcfg->entries[i].pci_bus_start; bus < mcfg->entries[i].pci_bus_end; bus++) {
			void* bus_address = (void*) (mcfg->entries[i].base_address + (bus << 20));

			foreach (device, 32) {
				void* dev_addr = (void*) (bus_address + (device << 15));
				pci_hdr_t* dev = dev_addr;

				if (dev->vendor == 0xffff) continue;

				foreach (f, 8) {
					pci_hdr_t* func = (pci_hdr_t*) (dev_addr + (f << 12));
					
					if (func->vendor == 0 || func->vendor == 0xffff) continue;

					pci_add_device(func);
				}
			}
		}
	}
}
