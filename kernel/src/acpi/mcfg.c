#include <acpi/mcfg.h>

void mcfg_parse(mcfg_t* mcfg) {
	u32 entries = (mcfg->base.len - sizeof(mcfg_t)) / sizeof(mcfg_entry_t);

	foreach (i, entries) {
		for (u32 bus = mcfg->entries[i].pci_bus_start; bus < mcfg->entries[i].pci_bus_end; bus++) {
			void* bus_address = (void*) (mcfg->entries[i].base_address + (bus << 20));
			pci_hdr_t* bus = (pci_hdr_t*) bus_address;

			if (bus->device == 0 || bus->device == 0xffff) continue;

			foreach (device, 32) {
				void* dev_addr = (void*) (bus_address + (device << 15));
				pci_hdr_t* dev = (pci_hdr_t*) dev_addr;

				if (dev->device == 0 || dev->device == 0xffff) continue;

				foreach (func, 8) {
					void* func_addr = (void*) (dev_addr + (func << 12));
					pci_hdr_t* func = (pci_hdr_t*) func_addr;

					if (func->device == 0 || func->device == 0xffff) continue;

					printk("%04x %04x\n", func->vendor, func->device);
				}
			}
		}
	}
}
