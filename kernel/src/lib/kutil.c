#include "kutil.h"

idtr idt;

// void setup_paging() {
	// pml4 = (page_map_level*)request_page();
	// memset(pml4, 0, sizeof(page_map_level));

	// for (uint64_t i = 0; i < total_mem; i += 0x1000) {
	// 	map_address((void*)i, (void*)i);
	// 	//printk("hahhah");
	// }

	// //printk("...\n");

	// uint64_t fb_base = (uint64_t)graphics->framebuffer->BaseAddress;
	// uint64_t fb_size = (uint64_t)graphics->framebuffer->BufferSize + 0x1000;
	// for (uint64_t i = fb_base; i < fb_base + fb_size; i += 0x1000) {
	// 	map_address((void*)i, (void*)i);
	// }

	// asm("mov %0, %%cr3" : : "r" (pml4));
//}

void setup_font() {
	if(*((uint8_t*)&_binary_font_psf_start) == 0x36) {
		serial_write(0x3f8, "Recognized psf ver. 1!\n\r");
		psf1_header* _tmp = (psf1_header*) &_binary_font_psf_start;
		def->numglyph = 512; // If extended, 256 otherwise
		def->height = 16;
		def->width = 8;
		def->bytesperglyph = 16;
		def->magic = ((_tmp->magic1) | (_tmp->magic2 >> 8));
		glyph_buffer = (void*)((uint64_t)&_binary_font_psf_start + sizeof(psf1_header));
	} else if (*((uint8_t*)&_binary_font_psf_start) == 0x72) {
		serial_write(0x3f8, "Recognized psf ver. 2!\n\r");
		psf2_header* tmp = (psf2_header*) &_binary_font_psf_start;
		memcpy(def, tmp, sizeof(psf2_header));
		glyph_buffer = (void*)((uint64_t)&_binary_font_psf_start + sizeof(psf2_header));
	} else {
		serial_write(0x3f8, "Bad font!\n\r");
	}
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

void kinit(system_info* _info) {
	info = _info;
	clear_screen();
	setup_font();

	text_color(0x0000ffcc);
	printk("Framebuffer resolution: %ud x %ud\n", info->g_info->info->width, info->g_info->info->height);
	text_color_reset();

	map_memory();
	init_bitmap(bm);

	struct gdt_descriptor desc;
	desc.size = sizeof(struct gdt) - 1;
	desc.offset = (uint64_t) &gdt_obj;
	load_gdt(&desc);

	int_prep();
}
