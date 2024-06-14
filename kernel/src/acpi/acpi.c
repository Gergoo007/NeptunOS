#include <acpi/acpi.h>

rsdp* _rsdp;

sdt_base* acpi_get_table(const char* name) {
	// RSDT
	sdt_base* _rsdt = (sdt_base*)(u64)_rsdp->rsdt_addr;

	u64 num_entries = (_rsdt->len - sizeof(sdt_base)) / 4;
	for (u64 i = 0; i < num_entries; i++) {
		sdt_base* table = (sdt_base*)(u64)((rsdt*)_rsdt)->ptrs[i];
		if (!strncmp(table->signature, name, 4))
			return table;
	}

	return NULL;
}

void acpi_init(void* tag) {
	_rsdp = &((mb_tag_rsdp*)tag)->rsdp;

	if (strncmp(_rsdp->signature, "RSD PTR ", 8))
		error("Ervenytelen rsdp/XSDT alairas!");

	madt* _madt = (madt*)acpi_get_table("APIC");
	if (_madt) {
		madt_parse(_madt);
	} else {
		error("MADT nincs!");
	}

	ioapic_write_entry(irq_to_gsi(IRQ_PIT), 0x20);
	ioapic_write_entry(irq_to_gsi(IRQ_KB), 0x21);
	ioapic_set_mask(irq_to_gsi(IRQ_KB), 1);
	// ioapic_write_entry(irq_to_gsi(IRQ_MOUSE), 0x22);

	mcfg* _mcfg = (mcfg*)acpi_get_table("MCFG");
	if (_mcfg) {
		mcfg_parse(_mcfg);
	} else {
		error("MCFG nincs!");
	}

	asm volatile ("sti");
}
