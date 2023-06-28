#include "apic.h"
#include "lapic.h"
#include <neptunos/memory/paging.h>
#include <neptunos/serial/com.h>

u16 num_cpus;
cpu_core_t* cpus;
void* lapic_addr;

void apic_init(void) {
	// while(1);
	// Disable PIC
	outb(0xff, 0x0021);
	outb(0xff, 0x00a1);

	lapic_addr = (void*)(u64)madt->lapic_addr;

	// Initialize CPU list
	num_cpus = 0;
	cpus = request_page();

	u8 lapic_nmi;

	for(madt_entry_base_t* entry = (madt_entry_base_t*)((u8*)madt + 44);
	(u8*)entry < (u8*)madt+madt->hdr.length;
	entry = (madt_entry_base_t*)((u8*)entry + entry->length)) {
		switch (entry->type) {
			case MADT_ENTRY_LAPIC:
				num_cpus++;
				cpus[num_cpus].apic_id = ((madt_entry_lapic_t*)entry)->apic_id;
				cpus[num_cpus].cpu_id = ((madt_entry_lapic_t*)entry)->cpu_id;
				cpus[num_cpus].cpu_enabled = ((madt_entry_lapic_t*)entry)->cpu_enabled;
				cpus[num_cpus].online_capable = ((madt_entry_lapic_t*)entry)->online_capable;
				printk("Found LAPIC/core\n");
				break;
			case MADT_ENTRY_IOAPIC:
				printk("Found IOAPIC\n");
				break;
			case MADT_ENTRY_LOCAL_X2APIC:
				printk("Found local X2APIC\n");
				break;
			case MADT_ENTRY_LAPIC_ADDR_OVERR:
				lapic_addr = (void*)((madt_entry_lapic_addr_override_t*)entry)->new_base;
				break;
			case MADT_ENTRY_LAPIC_NMI:
				if (((madt_entry_lapic_nmi_t*)entry)->acpi_cpu_uid == 255) {
					printk("Setting LINT\n");
					lapic_nmi = ((madt_entry_lapic_nmi_t*)entry)->lapic_lint;
				}
				break;
		}
	}
	printk("LAPIC addr: %p\n", lapic_addr);
	printk("Total number of cores: %d\n", num_cpus);
	for (u8 i = 0; i < num_cpus; i++) {
		printk("Active core: %d\n", cpus[i].cpu_enabled);
		printk("Online-capable core: %d\n", cpus[i].online_capable);
		printk("APIC ID: %d\n", cpus[i].apic_id);
		printk("CPU ID: %d\n", cpus[i].cpu_id);
	}

	asm("sti");

	map_page(lapic_addr, lapic_addr, MAP_FLAGS_IO_DEFAULTS);

	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) lapic_addr;
	printk("Lapic version: %08x\n", lapic->lapic_ver.max_lvt_entry);

	u32 reg = lapic->spurious_int_vector;
	reg = 0xff;
	reg |= 0x100;
	lapic->spurious_int_vector = reg;

	lapic->tpr = 0;

	lapic->timer_divide_config = 0b0001;
	
	lvt_entry_t entry = lapic->lvt_timer;
	entry.active_low = 0;
	entry.mask = 0;
	entry.vector_num = 0x51;
	entry.lvl_triggered = 1;
	entry.timer_mode = 0b01; // periodic
	lapic->lvt_timer = entry;

	lapic->timer_initial_count = 0x6000000;

	entry = lapic->lvt_lint0;
	entry.mask = 1;
	entry.nmi = lapic_nmi == 0 ? 0b100 : 0;
	lapic->lvt_lint0 = entry;

	entry = lapic->lvt_lint1;
	entry.mask = 1;
	entry.nmi = lapic_nmi ? 0b100 : 0;
	lapic->lvt_lint1 = entry;

	printk("Current count: %16x\n", lapic->timer_current_count);
	printk("Initial count: %16x\n", lapic->timer_initial_count);
}
