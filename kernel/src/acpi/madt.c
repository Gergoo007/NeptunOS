#include <acpi/madt.h>

u64 lapic_addr;
u8 dual_pic;

cpu_core* cores;
u16 num_cores;

void madt_parse(madt* _madt) {
	lapic_addr = _madt->lapic_addr;
	dual_pic = _madt->dual_pic;

	u64 table_end = (u64)_madt + _madt->base.len;
	madt_entry_base* entry = (madt_entry_base*) ((u8*)_madt + sizeof(madt));

	// sysinfo kitöltése
	cores = request_page();
	memset(cores, 0, PAGESIZE);
	num_cores = 0;

	ioapics = request_page();
	memset(ioapics, 0, PAGESIZE);
	num_ioapics = 0;

	// MADT entry type 1 might not be provided,
	// in that case use default settings
	ioapics[0].base = 0xfec00000;
	ioapics[0].gsi_base = 0;

	for (u8 i = 0; i < 16; i++) ioapic_redirs[i] = i;
	
	for (;((u64)entry + entry->size) != table_end;
		entry = (madt_entry_base*) ((u8*)entry + entry->size)) {
		
		switch (entry->type) {
			case MADT_ENTRY_LAPIC: {
				// Új processzor mag
				madt_entry_lapic* e = (madt_entry_lapic*)entry;
				cores[num_cores].acpi_cpu_id = e->acpi_cpu_id;
				cores[num_cores].apic_id = e->apic_id;
				cores[num_cores].cpu_enabled = e->cpu_enabled;
				cores[num_cores].online_cap = e->online_cap;

				num_cores++;

				break;
			}
			case MADT_ENTRY_IOAPIC: {
				madt_entry_ioapic* e = (madt_entry_ioapic*)entry;
				ioapics[num_ioapics].base = e->addr;
				ioapics[num_ioapics].gsi_base = e->gsi_base;

				num_ioapics++;

				break;
			}
			case MADT_ENTRY_X2APIC: {
				// madt_entry_x2apic_t* e = (madt_entry_x2apic_t*)entry;
				break;
			}
			case MADT_ENTRY_LAPIC_ADDR_OVERR: {
				madt_entry_lapic_addr_t* e = (madt_entry_lapic_addr_t*)entry;
				lapic_addr = e->new_addr;
				break;
			}
			case MADT_ENTRY_OVERR: {
				madt_entry_override* e = (madt_entry_override*)entry;
				ioapic_redirs[e->irq] = e->gsi;
				break;
			}
			default: {
				// report("MADT entry of type %d\n", entry->type);
				break;
			}
		}
	}

	lapic_init();
}
