#include <apic/lapic.h>

void lapic_init() {
	// Set EOI broadcasting, enable LAPIC
	u32 reg = 0xff;
	reg |= 0x100;
	reg &= ~(1 << 12);
	printk("reg: %p\n", reg);
	lapic_write_reg(0xf0, reg);
}

void lapic_write_reg(u32 offset, u32 value) {
	*(u32*)(lapic_addr + offset) = value;
}

u32 lapic_read_reg(u32 offset) {
	return *(u32*)(lapic_addr + offset);
}
