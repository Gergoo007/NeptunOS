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

u8 num_ioapic_overrides = 0;
ioapic_override_t* ioapic_overrides = 0;

void* lapic_base = 0;

void mask_pic(void) {
	// Disable PIC
	outb(0xff, 0x0021);
	outb(0xff, 0x00a1);
}

u32 ioapic_read_reg(ioapic_t* ioapic, u32 reg) {
	volatile u32* addr_reg = ioapic->addr;
	volatile u32* data_reg = (volatile u32*)((u64)ioapic->addr+0x10);
	
	*addr_reg = reg;
	return *data_reg;
}

void ioapic_write_reg(ioapic_t* ioapic, u32 reg, u32 val) {
	volatile u32* addr_reg = ioapic->addr;
	volatile u32* data_reg = (volatile u32*)((u64)ioapic->addr+0x10);
	
	*addr_reg = reg;
	*data_reg = val;
}

void ioapic_init(void) {
	// Overrides: translate an IRQ into a GSI (provided)
	// Redirection: forward the GSI to the specified LAPIC

	// Setup RTC redirection (IRQ 8)
	// This is the first IO APIC, and each one
	// can handle 24 GSIs, so if the GSI of IRQ 8 is less than
	// 24, this redirection entry goes to the first IO APIC

	u8 rtc_gsi = 8;
	for (u8 i = 0; i < num_ioapic_overrides; i++) {
		if (ioapic_overrides[i].irq == 8) {
			rtc_gsi = ioapic_overrides[i].gsi;
			break;
		}
	}

	enable_rtc();
	mask_pic();

	printk("RTC GSI: %d\n", rtc_gsi);

	ioapic_redirect_entry_t entry;
	map_page(ioapics[0].addr, ioapics[0].addr, MAP_FLAGS_IO_DEFAULTS | MAP_FLAG_WRITE_THROUGH);
	entry.vector = 0x78;
	entry.delivery_mode = 0;
	entry.logical_dest = 0;
	entry.active_low = 1;
	entry.lvl_triggered = 0;
	entry.mask = 0;
	entry.apic_id = cpuid_x2apic_id();
	ioapic_write_reg(&ioapics[0], 0x10 + (8 * 2), entry.first_part);
	ioapic_write_reg(&ioapics[0], 0x11 + (8 * 2) + 1, entry.second_part);
	
	entry.vector = 0x80;
	entry.delivery_mode = 0;
	entry.logical_dest = 0;
	entry.active_low = 1;
	entry.lvl_triggered = 0;
	entry.mask = 0;
	entry.apic_id = cpuid_x2apic_id();
	ioapic_write_reg(&ioapics[0], 0x12, entry.first_part);
	ioapic_write_reg(&ioapics[0], 0x13, entry.second_part);
}

void apic_init(void) {
	mask_pic();
	
	lapic_base = (void*)(u64)madt->lapic_addr;

	// Initialize CPU list
	num_cpus = 0;
	cpus = request_page();

	// Initialize IOAPIC list
	num_ioapics = 0;
	ioapics = request_page();

	// Initialize redirection list
	num_ioapic_overrides = 0;
	ioapic_overrides = request_page();

	u8 lapic_nmi;

	for(madt_entry_base_t* entry = (madt_entry_base_t*)((u8*)madt + 44);
	(u8*)entry < (u8*)madt+madt->hdr.length;
	entry = (madt_entry_base_t*)((u8*)entry + entry->length)) {
		switch (entry->type) {
			case MADT_ENTRY_LAPIC:
				cpus[num_cpus].apic_id = ((madt_entry_lapic_t*)entry)->apic_id;
				cpus[num_cpus].cpu_id = ((madt_entry_lapic_t*)entry)->cpu_id;
				cpus[num_cpus].cpu_enabled = ((madt_entry_lapic_t*)entry)->cpu_enabled;
				cpus[num_cpus].online_capable = ((madt_entry_lapic_t*)entry)->online_capable;
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
				ioapic_overrides[num_ioapic_overrides].irq = ent->irq_source;
				ioapic_overrides[num_ioapic_overrides].gsi = ent->gsi;
				num_ioapic_overrides++;
			}
				break;
			case MADT_ENTRY_LOCAL_X2APIC:
				printk("Found local X2APIC\n");
				break;
			case MADT_ENTRY_LAPIC_ADDR_OVERR:
				lapic_base = (void*)((madt_entry_lapic_addr_override_t*)entry)->new_base;
				break;
			case MADT_ENTRY_LAPIC_NMI:
				if (((madt_entry_lapic_nmi_t*)entry)->acpi_cpu_uid == 255) {
					lapic_nmi = ((madt_entry_lapic_nmi_t*)entry)->lapic_lint;
				}
				break;
		}
	}

	asm("sti");

	map_page(lapic_base, lapic_base, MAP_FLAGS_IO_DEFAULTS);

	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) lapic_base;
	printk("Lapic version: %08x\n", lapic->lapic_ver.max_lvt_entry);

	u32 reg = lapic->spurious_int_vector;
	reg = 0xff;
	reg |= 0x100;
	lapic->spurious_int_vector = reg;

	lapic->tpr = 0;

	lapic->timer_divide_config = 0b0001;
	
	lvt_entry_t entry = lapic->lvt_timer;
	entry.active_low = 0;
	entry.mask = 1;
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

	// for (u8 i = 0; i < num_ioapics; i++)
	ioapic_init();
}
