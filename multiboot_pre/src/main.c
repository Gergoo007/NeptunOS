#include "multiboot2.h"
#include "elf.h"
#include "pmm/mmap.h"
#include "pmm/pmm.h"
#include "serial.h"
#include "paging/paging.h"

extern void* _binary_out_post_start;
extern u64 _binary_out_post_size;
extern void* _binary_out_post_end;
extern void* p4_table;

tag_align __attribute__((section(".multiboot"))) multiboot_hdr_t header = {
	// Magic
	0xE85250D6,
	// Arch
	0,
	// Hdr length
	sizeof(multiboot_hdr_t),
	// Checksum (magic fields + checksum = 0)
	(u32)(0-0xE85250D6-0-sizeof(multiboot_hdr_t)),

	.fb_tag = {
		5,		// Type
		0,		// Flags
		20,		// Size
		1280,	// Width
		720,	// Height
		32,		// BPP (bits, not bytes)
	},

	.end_tag = {
		0,
		0,
		0,
	},
};

void c_main(multiboot_hdr_t* mbi) {
	multiboot_tag_t* tag = (multiboot_tag_t*)((u8*)mbi+8);
	
	multiboot_tag_framebuffer_t* fb_tag;
	for (; tag->type != MULTIBOOT_TAG_TYPE_END;
	tag = (multiboot_tag_t*) ((u8*) tag + ((tag->size + 7) & ~7))) {
		switch (tag->type) {
			case MULTIBOOT_TAG_TYPE_EFI_MMAP: {
				interpret_mmap((multiboot_tag_efi_mmap_t*)tag);
				break;
			}
			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
				fb_tag = (multiboot_tag_framebuffer_t*) tag;
				break;
			}
		}
	}

	// If the program didn't froze, there should be a white square on the screen
	for (u8 x = 0; x < 100; x++) {
		for (u8 y = 0; y < 100; y++) {
			*((u32*) (((u64) fb_tag->common.framebuffer_addr) + fb_tag->common.framebuffer_pitch*y + (fb_tag->common.framebuffer_bpp/8)*x)) = 0x00ffffff;
		}
	}

	__asm__("movq $0xC0000011, %rax");
	__asm__("mov %rax, %cr0");

	__asm__("movq $0x00000768, %rax");
	__asm__("movq %rax, %cr4");

	setup_paging();

	// Kernel is to be loaded at 0xFFFFFF8000000000
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)&_binary_out_post_start;
	if (memcmp(ehdr->e_ident, "\127ELF", 3)) {
		printk("Kernel found!\n\r");
	} else {
		printk("No kernel found!\n\r");
	}

	__asm__("cli\n hlt");
}

void __stack_chk_fail() {}
