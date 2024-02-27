#include <acpi/acpi.h>

rsdp_t* rsdp;

sdt_base_t* acpi_get_table(const char* name) {
	// RSDT
	sdt_base_t* rsdt = (sdt_base_t*)(u64)rsdp->rsdt_addr;

	u64 num_entries = (rsdt->len - sizeof(sdt_base_t)) / 4;
	for (u64 i = 0; i < num_entries; i++) {
		sdt_base_t* table = (sdt_base_t*)(u64)((rsdt_t*)rsdt)->ptrs[i];
		if (!strncmp(table->signature, name, 4))
			return table;
	}

	return NULL;
}

void acpi_init(void* tag) {
	rsdp = &((mb_tag_rsdp_t*)tag)->rsdp;

	if (strncmp(rsdp->signature, "RSD PTR ", 8))
		printk("Ervenytelen rsdp/XSDT alairas!\n");

	madt_t* madt = (madt_t*)acpi_get_table("APIC");
	if (madt) {
		madt_parse(madt);
	} else {
		printk("E: MADT nincs!\n");
	}

	ioapic_write_entry(irq_to_gsi(IRQ_PIT), 0x20);
	// ioapic_set_mask(irq_to_gsi(IRQ_PIT), 1);
	ioapic_write_entry(irq_to_gsi(IRQ_KB), 0x21);
	ioapic_write_entry(irq_to_gsi(IRQ_MOUSE), 0x22);

	// ioapic_write_entry(irq_to_gsi(IRQ_PIT), 0x21);

	asm volatile ("sti");
}
