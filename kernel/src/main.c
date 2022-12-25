#include "lib/globals.h"
#include "lib/graphics/graphics.h"

#include "lib/graphics/text_renderer.h"
#include "lib/serial/serial.h"
#include "lib/libk/string.h"

#include "lib/memory/memory.h"

#include "lib/tables/gdt.h"
#include "lib/tables/idt.h"
#include "lib/tables/interrupts.h"

#include "lib/power/power.h"

BootInfo* graphics;
uint16_t cursor_x = 0, cursor_y = 0;

idtr idt;

void kinit(BootInfo* restrict _graphics);
void int_prep();

uint8_t kmain(BootInfo *restrict _graphics) {
    kinit(_graphics);

	

	text_color(0x0000ff00);
	printk("Successfully reached end of kernel kmain(), halting...\n");
	halt();	
	printk("Halt unsuccessful.\n");

	return 0;
}

void add_interrupt(void* handler, uint8_t type_attrib, uint8_t num) {
	idt_desc_entry* page_flt = (idt_desc_entry*)(idt.offset + (num * sizeof(idt_desc_entry)));
	memset(page_flt, 0, sizeof(idt_desc_entry));
	set_offset((uint64_t)handler, page_flt);
	page_flt->attrib = type_attrib;
	page_flt->selector = 0x08;
}

void int_prep() {
	idt.limit = 0x0FFF;
	idt.offset = (uint64_t)request_page();

	add_interrupt(page_flt_handler, IDT_TA_INTERRUPT_GATE, 0x0e);
	add_interrupt(double_flt_handler, IDT_TA_INTERRUPT_GATE, 0x08);
	add_interrupt(general_protection_handler, IDT_TA_INTERRUPT_GATE, 0x0d);
	add_interrupt(invalid_opcode_flt_handler, IDT_TA_INTERRUPT_GATE, 0x06);
	add_interrupt(custom_handler, IDT_TA_INTERRUPT_GATE, 0x30);

	asm("lidt %0" : : "m" (idt));
}

void kinit(BootInfo* restrict _graphics) {
	graphics = _graphics;

	clear_screen();

	text_color_push(0x0000ffcc);
	printk("Framebuffer resolution: %ud x %ud\n", graphics->framebuffer->Width, graphics->framebuffer->Height);
	text_color_pop();

	map_memory();
	init_bitmap(bm);

	printk("Kernel size: %ud KiB\n", ((uint64_t) &KEND - (uint64_t) &KSTART) / 1024);

	serial_init();

	struct gdt_descriptor desc;
	desc.size = sizeof(struct gdt) - 1;
	desc.offset = (uint64_t) &gdt_obj;
	load_gdt(&desc);

	printk("Total mem: %ud MiB or %ud GiB (%ud pages)\n", total_mem / 1024 / 1024, total_mem / 1024 / 1024 / 1024, num_pages);

	int_prep();
}
