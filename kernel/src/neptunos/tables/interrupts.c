#include <neptunos/acpi/apic/lapic.h>
#include <neptunos/acpi/apic/apic.h>
#include <neptunos/tables/interrupts.h>
#include <neptunos/graphics/text_renderer.h>

#include <neptunos/graphics/panic.h>

_attr_no_caller_saved_regs void __printk(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	_printk(fmt, args);
	va_end(args);
}

_attr_int void page_flt_handler(struct interrupt_frame* frame) {
	panic("Page fault detected!");
}

_attr_int void double_flt_handler(struct interrupt_frame* frame) {
	panic("Double fault detected!");
}

_attr_int void invalid_opcode_flt_handler(struct interrupt_frame* frame) {
	panic("Invalid opcode!");
}

_attr_int void general_protection_handler(struct interrupt_frame* frame) {
	panic("General protection exception!");
}

_attr_int void custom_handler(struct interrupt_frame* frame) {
	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) lapic_addr;
	__printk("Custom interrupt triggered!\n");

	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
}

_attr_int void spurious_int(struct interrupt_frame* frame) {
	// text_color_push(0x0000ff88);
	// printk("Custom interrupt triggered!");
	// text_color_pop();
	panic("Spurious int triggered!");
	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) lapic_addr;
	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
}

_attr_int void apic_test(struct interrupt_frame* frame) {
	// text_color_push(0x0000ff88);
	// printk("Custom interrupt triggered!\n");
	// text_color_pop();
	// panic("Custom handler triggered!");

	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) lapic_addr;
	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
}

u64 int_handlers[][2] = {
	{
		(u64)page_flt_handler,
		0xe
	},
	{
		(u64)double_flt_handler,
		0x8
	},
	{
		(u64)invalid_opcode_flt_handler,
		0x6
	},
	{
		(u64)general_protection_handler,
		0xd
	},
	{
		(u64)custom_handler,
		0x51
	},
	{
		(u64)spurious_int,
		0xff
	},
};
