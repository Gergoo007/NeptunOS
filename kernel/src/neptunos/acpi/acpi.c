#include <neptunos/acpi/acpi.h>
#include <neptunos/pci/pci.h>
#include <neptunos/libk/string.h>

sdt_hdr_t* xsdt = NULL;
mcfg_hdr_t* mcfg = NULL;

void init_acpi(void) {
	// xsdt = (sdt_hdr_t*)((rsdp_desc_2_t*)info->mem_info->rsdp)->xsdt_address;

	// mcfg = get_table_address("MCFG");
	// if (!mcfg) {
	// 	printk("Failed to get MCFG!\n");
	// 	return;
	// }

	// pci_enumerate();
}

void* get_table_address(char* table) {
	// uint32_t num_entries = (xsdt->length - sizeof(sdt_hdr_t)) / 8;
	// for (uint64_t i = 0; i < num_entries; i++) {
	// 	sdt_hdr_t* next_sdt_hdr = (sdt_hdr_t*) *((uint64_t*)((uint64_t)xsdt + sizeof(sdt_hdr_t) + (i * 8)));

	// 	if (strncmp(next_sdt_hdr->signature, table, 4)) {
	// 		return next_sdt_hdr;
	// 	}
	// }
	return NULL;
}
