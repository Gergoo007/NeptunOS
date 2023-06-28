#include <neptunos/acpi/acpi.h>
#include <neptunos/pci/pci.h>
#include <neptunos/libk/string.h>
#include <neptunos/acpi/apic/apic.h>

xsdp_t* xsdp = NULL;
sdt_hdr_t* xsdt = NULL;
mcfg_hdr_t* mcfg = NULL;
madt_hdr_t* madt = NULL;

void init_acpi(void) {
	// Identity map XSDT
	xsdt = (sdt_hdr_t*)xsdp->xsdt_addr;
	map_region((u8*)xsdt-0x1000, (u8*)xsdt-0x1000, xsdp->length/0x1000 + 1, MAP_FLAGS_DEFAULTS);

	u32 num_entries = (xsdt->length - sizeof(sdt_hdr_t)) / 8;
	for (u64 i = 0; i < num_entries; i++) {
		sdt_hdr_t* next_sdt_hdr = (sdt_hdr_t*) *((uint64_t*)((uint64_t)xsdt + sizeof(sdt_hdr_t) + (i * 8)));
		map_page(next_sdt_hdr, next_sdt_hdr, MAP_FLAGS_DEFAULTS);

		const char* sign = next_sdt_hdr->signature;
		if (strncmp(sign, "MCFG", 4)) {
			mcfg = (mcfg_hdr_t*)next_sdt_hdr;
			pci_enumerate();
		} else if (strncmp(sign, "APIC", 4)) {
			madt = (madt_hdr_t*)next_sdt_hdr;
			apic_init();
		}
	}
}
