#include "interrupts.h"
#include "../graphics/text_renderer.h"

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
