#include <neptunos/interrupts/apic/lapic.h>
#include <neptunos/interrupts/apic/apic.h>
#include <neptunos/interrupts/interrupts.h>
#include <neptunos/graphics/text_renderer.h>

#include <neptunos/serial/com.h>

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
	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) lapic_base;
	__printk("Custom interrupt triggered!\n");

	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
}

_attr_int void rtc_test(struct interrupt_frame* frame) {
	__printk("RTC int!\n");
	outb(0x70, 0x0C);	// select register C
	inb(0x71);		// just throw away contents
}

_attr_int void ps2_kb_press(struct interrupt_frame* frame) {
	// Just send an EOI for now
	// __printk("Key pressed! (%02x)\n", inb(0x60));

	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) lapic_base;

	// __printk("Return value: %d\n", wrapper(test));

	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
}

_attr_int void spurious_int(struct interrupt_frame* frame) {
	// text_color_push(0x0000ff88);
	// printk("Custom interrupt triggered!");
	// text_color_pop();
	panic("Spurious int triggered!");
	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) lapic_base;
	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
}

_attr_int void apic_test(struct interrupt_frame* frame) {
	// text_color_push(0x0000ff88);
	// printk("Custom interrupt triggered!\n");
	// text_color_pop();
	// panic("Custom handler triggered!");

	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) lapic_base;
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
		(u64)rtc_test,
		0x78
	},
	{
		(u64)ps2_kb_press,
		0x80
	},
	{
		(u64)spurious_int,
		0xff
	},
	// Mark end of array
	{
		(u64)0,
		(u64)0,
	}
};
