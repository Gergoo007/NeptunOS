#include <apic/apic.h>

ioapic_t* ioapics;
u16 num_ioapics;

// For 16 IRQs
u32 ioapic_redirs[16];

void lapic_init() {
	// Set EOI broadcasting, enable LAPIC
	u32 reg = 0xff;
	reg |= 0x100;
	reg &= ~(1 << 12);
	lapic_write_reg(0xf0, reg);

	pit_init();
}

void lapic_write_reg(u32 offset, u32 value) {
	*(u32*)(lapic_addr + offset) = value;
}

u32 lapic_read_reg(u32 offset) {
	return *(u32*)(lapic_addr + offset);
}

void lapic_eoi() {
	lapic_write_reg(0xb0, 0);
}

void ioapic_write(u16 reg, u32 val) {
	u32 volatile* addr = (u32*)(ioapics[0].base);
	u32 volatile* data = (u32*)(ioapics[0].base + 0x10);

	*addr = (reg & 0xff);
	*data = val;
}

u32 ioapic_read(u16 reg) {
	u32 volatile *addr = (u32 volatile *)ioapics[0].base;
	addr[0] = (reg & 0xff);
	return addr[4];
}

void ioapic_write_entry(u32 gsi, u8 vector) {
	ioapic_redir_entry_t base = {
		.vector = vector,
		.delivery = 0,
		.dest_mode = 0,
		.active_low = 0,
		.lvl_trig = 0,
		.mask = 0,
		.destination = 0,
	};

	gsi *= 2;
	ioapic_write(0x10 + gsi, base.first_part);
	ioapic_write(0x10 + gsi + 1, base.second_part);
}

void ioapic_set_mask(u32 gsi, u8 mask) {
	gsi *= 2;
	ioapic_redir_entry_t entry;
	entry.first_part = ioapic_read(0x10 + gsi);
	entry.mask = mask;
	ioapic_write(0x10 + gsi, entry.first_part);
}
