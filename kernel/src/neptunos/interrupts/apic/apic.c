#include "apic.h"
#include "lapic.h"
#include "ioapic.h"
#include <neptunos/time/rtc.h>
#include <neptunos/memory/paging.h>
#include <neptunos/serial/com.h>

u16 num_cpus = 0;
cpu_core_t* cpus = 0;

u8 num_ioapics = 0;
ioapic_t* ioapics = 0;

void mask_pic(void) {
	// Disable PIC
	outb(0xff, 0x21);
	io_wait(2);
	outb(0xff, 0xa1);
	io_wait(2);
}

void apic_init(void) {
	mask_pic();

	// Initialize CPU list
	num_cpus = 0;
	cpus = request_page();

	// Initialize IOAPIC list
	num_ioapics = 0;
	ioapics = request_page();

	// Initialize redirection list
	num_ioapic_overrides = 0;
	ioapic_overrides = request_page();
	for (u8 i = 0; i < 16; i++)
		ioapic_overrides[i] = 0xff;

	void* new_lapic_base = NULL;
	for(madt_entry_base_t* entry = (madt_entry_base_t*)((u8*)madt + 44);
	(u8*)entry < (u8*)madt+madt->hdr.length;
	entry = (madt_entry_base_t*)((u8*)entry + entry->length)) {
		switch (entry->type) {
			case MADT_ENTRY_LAPIC:
				cpus[num_cpus].apic_id = ((madt_entry_lapic_t*)entry)->apic_id;
				cpus[num_cpus].cpu_id = ((madt_entry_lapic_t*)entry)->cpu_id;
				cpus[num_cpus].cpu_enabled = ((madt_entry_lapic_t*)entry)->cpu_enabled;
				cpus[num_cpus].online_capable = ((madt_entry_lapic_t*)entry)->online_capable;
				cpus[num_cpus].lapic_base = (void*)(u64)madt->lapic_addr;
				num_cpus++;
				break;
			case MADT_ENTRY_IOAPIC: {
				madt_entry_ioapic_t* ent = (madt_entry_ioapic_t*)entry;
				ioapics[num_ioapics].addr = (void*)(u64)ent->ioapic_addr;
				ioapics[num_ioapics].ioapic_id = ent->ioapic_id;
				ioapics[num_ioapics].gis_base = ent->gsi_base;
				num_ioapics++;
				break;
			}
			case MADT_ENTRY_IOAPIC_OVERR: {
				madt_entry_int_override_t* ent = (madt_entry_int_override_t*)entry;
				ioapic_overrides[ent->irq_source] = ent->gsi;
				num_ioapic_overrides++;
				break;
			}
			case MADT_ENTRY_LOCAL_X2APIC:
				printk("Found local X2APIC\n");
				break;
			case MADT_ENTRY_LAPIC_ADDR_OVERR:
				new_lapic_base = (void*)((madt_entry_lapic_addr_override_t*)entry)->new_base;
				break;
			case MADT_ENTRY_LAPIC_NMI:
				if (((madt_entry_lapic_nmi_t*)entry)->acpi_cpu_uid == 255) {
					cpus[0].nmi_pin = ((madt_entry_lapic_nmi_t*)entry)->lapic_lint;
				}
				break;
		}
	}
	// Set the new LAPIC base for all processors
	if(new_lapic_base) {
		for (u8 i = 0; i < num_cpus; i++)
			cpus[i].lapic_base = new_lapic_base;
	}

	lapic_init(&cpus[0]);
	ioapic_init();
}
