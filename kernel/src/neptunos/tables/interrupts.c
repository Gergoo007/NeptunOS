#include <neptunos/tables/interrupts.h>
#include <neptunos/hw_interrupts/pic.h>
#include <neptunos/hw_interrupts/pit.h>
#include <neptunos/graphics/text_renderer.h>
#include <neptunos/hw_interrupts/kb.h>

#include <neptunos/graphics/panic.h>

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
	// text_color_push(0x0000ff88);
	// printk("Custom interrupt triggered!");
	// text_color_pop();
	panic("Custom handler triggered!");
}

_attr_no_caller_saved_regs void display_pressed(uint8_t scan_code) {
	if (!(scan_code > 0x80)) { // Released scancodes are not implemented
		render_char(translate(scan_code));
	}
}

_attr_int void pic_kb_press(struct interrupt_frame* frame) {
	uint8_t scan_code;
	_in8(0x60, &scan_code);
	display_pressed(scan_code);
	pic_end_slave();
}

_attr_int void pit_tick_int(struct interrupt_frame* frame) {
	// pit_tick();
	pic_end_slave();
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
		0x50
	},
};
