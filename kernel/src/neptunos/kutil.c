#include <neptunos/kutil.h>

void setup_paging(void) {
	asm volatile("movq %cr3, %rax");
	asm volatile("movq %%rax, %0" : "=m"(pml4));
}

void setup_font(void) {
	if(*((uint8_t*)&_binary_font_psf_start) == 0x36) {
		debug_puts("Recognized psf ver. 1!\n\r");
		psf1_header_t* tmp = (psf1_header_t*) &_binary_font_psf_start;
		def.numglyph = 512; // If extended, 256 otherwise
		def.height = 16;
		def.width = 8;
		def.bytesperglyph = 16;
		def.magic = ((tmp->magic1) | (tmp->magic2 >> 8));
		glyph_buffer = (void*)((uint64_t)&_binary_font_psf_start + sizeof(psf1_header_t));
	} else if (*((uint8_t*)&_binary_font_psf_start) == 0x72) {
		debug_puts("Recognized psf ver. 2!\n\r");
		psf2_header_t* tmp = (psf2_header_t*) &_binary_font_psf_start;
		memcpy(&def, tmp, sizeof(psf2_header_t));
		glyph_buffer = (void*)((uint64_t)&_binary_font_psf_start + sizeof(psf2_header_t));
	} else {
		debug_puts("Bad font!\n\r");
	}
}

void kinit(void) {
	memcpy = memcpy_comp;
	// Configure processor
	register u64 rax asm("%rax");
	if (cpuid_sse()) {
		serprintk("Enabling SSE\n\r");
		// Enable SSE
		// Disable x87 (bit 2 of CR0)
		asm volatile("mov %%cr0, %0" : "=a"(rax));
		rax &= ~(1 << 2);
		rax |= (1 << 1);
		asm volatile("mov %0, %%cr0" :: "a"(rax));

		asm volatile("mov %%cr4, %0" : "=a"(rax));
		rax |= (1 << 9);
		rax |= (1 << 10);
		rax |= (1 << 18);
		asm volatile("mov %0, %%cr4" :: "a"(rax));

		memcpy = memcpy_sse;
	}

	if (cpuid_avx2()) {
		serprintk("Enabling AVX2\n\r");

		asm volatile("mov %%cr0, %0" : "=a"(rax));
		rax &= ~(1 << 2);
		asm volatile("mov %0, %%cr0" :: "a"(rax));
		serprintk("asd123\n\r");
		// Correct XCR0
		asm volatile("xgetbv");
		rax |= 7;
		asm volatile("xsetbv");

		memcpy = memcpy_avx2;
	}

	// Process multiboot tags
	multiboot_tag_t* tag = (multiboot_tag_t*)((u8*)info->mb+8);
	
	for (; tag->type != MULTIBOOT_TAG_TYPE_END;
	tag = (multiboot_tag_t*) ((u8*) tag + ((tag->size + 7) & ~7))) {
		switch (tag->type) {
			case MULTIBOOT_TAG_TYPE_EFI_MMAP: {
				map_memory();
				break;
			}
			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
				multiboot_tag_framebuffer_t* fb_tag = (multiboot_tag_framebuffer_t*)(u64)tag;
				screen.fb_base = (void*)0xFFFFFF8040000000;
				screen.width = fb_tag->common.framebuffer_width;
				screen.height = fb_tag->common.framebuffer_height;
				screen.pitch = fb_tag->common.framebuffer_pitch;
				screen.bpp = fb_tag->common.framebuffer_bpp;

				serprintk("Base: %p\n\rWidth: %d\n\rHeight: %d\n\rPitch: %d\n\rBPP: %d\n\r",
					screen.fb_base, screen.width, screen.height, screen.pitch, screen.bpp);
				break;
			}
			case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
				multiboot_tag_acpi_new_t* acpi_tag = (multiboot_tag_acpi_new_t*) tag;
				xsdp = (xsdp_t*)acpi_tag->rsdp;
				break;
			}
		}
	}

	setup_font();
	cursor_x = cursor_y = 0;

	// void* stack_base = malloc(mib_bytes(4));
	// asm("movq %0, %%rsp" : "=r" (stack_base));

	#ifdef USE_DOUBLE_BUFFERING
		setup_back_buffer();
	#endif

	gdt_descriptor desc;
	desc.size = sizeof(gdt) - 1;
	desc.offset = (uint64_t) &gdt_obj;
	load_gdt(&desc);

	idt_init();

	setup_paging();

	// // remap_pic(0x20, 0x28);
	// // Unmask interrupts
	// // outb(0b11111100, PIC_M_DATA);
	// // outb(0b11111111, PIC_S_DATA);

	init_acpi();

	// asm("sti");

	// // Return address was saved on the stack, so we have to manually jump
	// void (*after_kinit)(void) = (void*)addr;
	// after_kinit();
}
