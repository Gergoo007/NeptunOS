#include <neptunos/interrupts/apic/lapic.h>
#include <neptunos/interrupts/apic/apic.h>
#include <neptunos/interrupts/interrupts.h>
#include <neptunos/graphics/text_renderer.h>

#include <neptunos/serial/com.h>

#include <neptunos/graphics/panic.h>

#include <neptunos/interrupts/pic/pic.h>

#include <neptunos/time/pit.h>

_attr_no_caller_saved_regs void safe_printk(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	_printk(fmt, args);
	va_end(args);
}

_attr_no_caller_saved_regs void safe_outb(u8 data, u16 port) { outb(data, port); }
_attr_no_caller_saved_regs u8 safe_inb(u16 port) { return inb(port); }

_attr_int void page_flt_handler(int_frame_t* frame) {
	panic("Page fault detected!");
}

_attr_int void double_flt_handler(int_frame_t* frame) {
	panic("Double fault detected!");
}

_attr_int void invalid_opcode_flt_handler(int_frame_t* frame) {
	panic("Invalid opcode!");
}

_attr_int void general_protection_handler(int_frame_t* frame) {
	safe_printk("err_code: %p, RIP: %p\nCS: %p, FLAGS: %p\nRSP: %p, SS: %p\n", frame->err_code, frame->rip, frame->cs, frame->flags, frame->rsp, frame->ss);
	panic("General protection exception!");
}

_attr_int void custom_handler(int_frame_t* frame) {
	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) cpus[0].lapic_base;
	safe_printk("Custom interrupt triggered!\n");

	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
}

_attr_int void rtc_test(int_frame_t* frame) {
	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) cpus[0].lapic_base;
	safe_printk("RTC int!\n");
	safe_outb(0x0C, 0x70);	// select register C
	safe_inb(0x71);		// just throw away contents
	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
}

_attr_int void ps2_kb_press(int_frame_t* frame) {
	// Just send an EOI for now
	safe_printk("Key pressed! (%02x)\n", safe_inb(0x60));

	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) cpus[0].lapic_base;

	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
}

_attr_int void ps2_kb_press_pic(int_frame_t* frame) {
	// Just send an EOI for now
	safe_printk("Key pressed! (%02x)\n", safe_inb(0x60));

	safe_outb(PIC_EOI, MPIC_CMD);
}

_attr_int void spurious_int(int_frame_t* frame) {
	panic("Spurious int triggered!");
	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) cpus[0].lapic_base;
	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
}

_attr_int void apic_test(int_frame_t* frame) {
	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) cpus[0].lapic_base;
	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
}

_attr_int void nmi(int_frame_t* frame) {
	safe_printk("NMI triggered!\n");
	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) cpus[0].lapic_base;
	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
}

// Non-NMI LINT int
_attr_int void lint(int_frame_t* frame) {
	safe_printk("LINT triggered!\n");
	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) cpus[0].lapic_base;
	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
}

_attr_int void pit_tick(int_frame_t* frame) {
	safe_printk("PIT tick!\n");
	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) cpus[0].lapic_base;
	u32 eoi = lapic->eoi;
	eoi &= 0;
	lapic->eoi = eoi;
	safe_outb(PIC_EOI, MPIC_CMD);
}

// These interrupts are always needed, regardless of
// the peripherals attached
u64 exception_isrs[][2] = {
	{
		(u64)page_flt_handler,
		0x0e
	},
	{
		(u64)double_flt_handler,
		0x08
	},
	{
		(u64)invalid_opcode_flt_handler,
		0x06
	},
	{
		(u64)general_protection_handler,
		0x0d
	},
	{
		(u64)pit_tick,
		0x60
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
