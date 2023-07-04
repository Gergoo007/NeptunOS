#include "ioapic.h"

u8 num_ioapic_overrides = 0;
u32* ioapic_overrides;

u32 ioapic_find_gsi(u8 irq) {
	if(ioapic_overrides[irq] == 0xff)
		return irq;
	else
		return ioapic_overrides[irq];
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

void ioapic_add_redirection(u32 gsi, u8 vector, u8 lvl_trig, u8 active_low, u8 target_id) {
	ioapic_redirect_entry_t entry;
	entry.vector = vector;
	entry.delivery_mode = 0;
	entry.logical_dest = 0;
	entry.active_low = active_low;
	entry.lvl_triggered = lvl_trig;
	entry.mask = 1;
	entry.apic_id = target_id;
	ioapic_write_reg(&ioapics[0], 0x10 + (gsi * 2), entry.first_part);
	ioapic_write_reg(&ioapics[0], 0x10 + (gsi * 2) + 1, entry.second_part);
}

void ioapic_set_mask(u32 gsi, u8 mask) {
	ioapic_redirect_entry_t entry;
	entry.first_part = ioapic_read_reg(&ioapics[0], 0x10 + (gsi * 2));
	entry.mask = mask;
	ioapic_write_reg(&ioapics[0], 0x10 + (gsi * 2), entry.first_part);
}

void ioapic_init(void) {
	// Overrides: translate an IRQ into a GSI (provided)
	// Redirection: forward the GSI to the specified LAPIC

	// Setup RTC redirection (IRQ 8)
	// This is the first IO APIC, and each one
	// can handle 24 GSIs, so if the GSI of IRQ 8 is less than
	// 24, this redirection entry goes to the first IO APIC
	
	enable_rtc();

	map_page(ioapics[0].addr, ioapics[0].addr, MAP_FLAGS_IO_DEFAULTS | MAP_FLAG_WRITE_THROUGH);

	for (u8 i = 0; i < 24; i++) {
		ioapic_set_mask(i, 1);
	}
	
	ioapic_add_redirection(ioapic_find_gsi(IRQ_PS2_KB), idt_add_isr(ps2_kb_press), 0, 1, cpus[0].apic_id);
	ioapic_set_mask(ioapic_find_gsi(IRQ_PS2_KB), 0);
	ioapic_add_redirection(ioapic_find_gsi(IRQ_RTC), idt_add_isr(rtc_test), 0, 1, cpus[0].apic_id);
	ioapic_set_mask(ioapic_find_gsi(IRQ_RTC), 0);
}
