#include <neptunos/acpi/acpi.h>
#include <neptunos/libk/string.h>

void init_acpi() {
	sdt_hdr_t* xsdt = (sdt_hdr_t*)((rsdp_desc_2_t*)info->mem_info->rsdp)->xsdt_address;
	uint32_t num_entries = (xsdt->length - sizeof(sdt_hdr_t)) / 8;

	for (uint64_t i = 0; i < num_entries; i++) {
		sdt_hdr_t* next_sdt_hdr_t = (sdt_hdr_t*) *((uint64_t*)((uint64_t)xsdt + sizeof(sdt_hdr_t) + (i * 8)));
	}
}
