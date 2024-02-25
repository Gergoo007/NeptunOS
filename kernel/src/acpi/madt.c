#include <acpi/madt.h>

u64 lapic_addr;
u8 dual_pic;

cpu_core_t* cores;
u16 num_cores;

void madt_parse(madt_t* madt) {
	lapic_addr = madt->lapic_addr;
	dual_pic = madt->dual_pic;

	u64 table_end = (u64)madt + madt->base.len;
	madt_entry_base_t* entry = (madt_entry_base_t*) ((u8*)madt + sizeof(madt_t));

	// sysinfo kitöltése
	cores = request_page();
	memset(cores, 0, PAGESIZE);
	num_cores = 0;

	ioapics = request_page();
	memset(ioapics, 0, PAGESIZE);
	num_ioapics = 0;

	for (u8 i = 0; i < 16; i++) ioapic_redirs[i] = i;
	
	for (;((u64)entry + entry->size) != table_end;
		entry = (madt_entry_base_t*) ((u8*)entry + entry->size)) {
		switch (entry->type) {
			case MADT_ENTRY_LAPIC: {
				// Új processzor mag
				madt_entry_lapic_t* e = (madt_entry_lapic_t*)entry;
				printk("Found core %d, active %d; capable %d\n", e->acpi_cpu_id, e->cpu_enabled, e->online_cap);

				cores[num_cores].acpi_cpu_id = e->acpi_cpu_id;
				cores[num_cores].apic_id = e->apic_id;
				cores[num_cores].cpu_enabled = e->cpu_enabled;
				cores[num_cores].online_cap = e->online_cap;

				num_cores++;

				break;
			}
			case MADT_ENTRY_IOAPIC: {
				madt_entry_ioapic_t* e = (madt_entry_ioapic_t*)entry;
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
				madt_entry_override_t* e = (madt_entry_override_t*)entry;
				ioapic_redirs[e->irq] = e->gsi;
				break;
			}
			default: {
				printk("MADT entry of type %d\n", entry->type);
				break;
			}
		}
	}

	lapic_init();
}
