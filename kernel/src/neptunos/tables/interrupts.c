#include <neptunos/tables/interrupts.h>
#include <neptunos/hw_interrupts/pic.h>
#include <neptunos/graphics/text_renderer.h>
#include <neptunos/hw_interrupts/kb.h>

__attribute__((interrupt)) void page_flt_handler(struct interrupt_frame* frame) {
	text_color_push(0x00ff0000);
	printk("Page fault detected!\n");
	text_color_pop();
}

__attribute__((interrupt)) void double_flt_handler(struct interrupt_frame* frame) {
	text_color_push(0x00ff0000);
	printk("Double fault detected!\n");
	text_color_pop();
}

__attribute__((interrupt)) void invalid_opcode_flt_handler(struct interrupt_frame* frame) {
	text_color_push(0x00ff0000);
	printk("Invalid opcode!\n");
	text_color_pop();
}

__attribute__((interrupt)) void general_protection_handler(struct interrupt_frame* frame) {
	text_color_push(0x00ff0000);
	printk("General protection exception!\n");
	text_color_pop();
}

__attribute__((interrupt)) void custom_handler(struct interrupt_frame* frame) {
	text_color_push(0x0000ff88);
	printk("Custom interrupt triggered!\n");
	text_color_pop();
}

__attribute__((interrupt)) void pic_kb_press(struct interrupt_frame* frame) {
	uint8_t scan_code = _in8(0x60);
	if (!(scan_code > 0x80)) // Released scancodes are not implemented
		render_char(translate(scan_code));
	pic_end_slave();
}
