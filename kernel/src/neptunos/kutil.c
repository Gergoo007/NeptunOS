#include <neptunos/kutil.h>

void setup_paging() {
	pml4 = (page_map_level*) request_page();
	memset(pml4, 0, 0x1000);

	// Identity map the whole physical memory
	map_region(0, 0, total_mem / 0x1000, MAP_FLAGS_DEFAULTS);

	// Identity map the framebuffer so it doesn't break
	map_region(fb_base,
		fb_base, bootboot.fb_size/0x1000, MAP_FLAGS_IO_DEFAULTS);

	asm("mov %0, %%cr3" :: "r" (pml4));
}

void setup_font() {
	if(*((uint8_t*)&_binary_font_psf_start) == 0x36) {
		serial_write(0x3f8, "Recognized psf ver. 1!\n\r");
		psf1_header_t* tmp = (psf1_header_t*) &_binary_font_psf_start;
		def->numglyph = 512; // If extended, 256 otherwise
		def->height = 16;
		def->width = 8;
		def->bytesperglyph = 16;
		def->magic = ((tmp->magic1) | (tmp->magic2 >> 8));
		glyph_buffer = (void*)((uint64_t)&_binary_font_psf_start + sizeof(psf1_header_t));
	} else if (*((uint8_t*)&_binary_font_psf_start) == 0x72) {
		serial_write(0x3f8, "Recognized psf ver. 2!\n\r");
		psf2_header_t* tmp = (psf2_header_t*) &_binary_font_psf_start;
		memcpy(def, tmp, sizeof(psf2_header_t));
		glyph_buffer = (void*)((uint64_t)&_binary_font_psf_start + sizeof(psf2_header_t));
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
	add_interrupt(pic_kb_press, IDT_TA_INTERRUPT_GATE, 0x21);
	add_interrupt(pit_tick_int, IDT_TA_INTERRUPT_GATE, 0x20);

	asm("lidt %0" : : "m" (idt));
}

void kinit() {
	fb_base = &fb;

	setup_font();

	serial_init();

	map_memory();
	init_bitmap(bm);

	printk("Done\n");

	// // Save return address
	// uint64_t addr = (uint64_t)__builtin_extract_return_addr(__builtin_return_address(0));

	// void* stack_base = malloc(mib_bytes(4));
	// asm("movq %0, %%rsp" : "=r" (stack_base));

	// #ifdef USE_DOUBLE_BUFFERING
	// 	setup_back_buffer();
	// #endif

	// clear_screen();

	// text_color(0x0000ffcc);
	// printk("Framebuffer resolution: %ud x %ud\n", bootboot.fb_width, bootboot.fb_height);
	// printk("Total memory: %ud MiB\n", total_mem / 1024 / 1024);
	// text_color_reset();

	// gdt_descriptor desc;
	// desc.size = sizeof(gdt) - 1;
	// desc.offset = (uint64_t) &gdt_obj;
	// load_gdt(&desc);

	// int_prep();

	// setup_paging();

	// remap_pic(0x20, 0x28);
	// // Unmask interrupts
	// outb(0b11111100, PIC_M_DATA);
	// outb(0b11111111, PIC_S_DATA);

	// init_acpi();

	// asm("sti");

	// // Return address was saved on the stack, so we have to manually jump
	// void (*after_kinit)(void) = (void*)addr;
	// after_kinit();
}
