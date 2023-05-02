#include "multiboot2.h"
#include "elf.h"
#include "pmm/mmap.h"
#include "pmm/pmm.h"
#include "serial.h"

extern void* _binary_out_post_start;
extern uint64_t _binary_out_post_size;
extern void* _binary_out_post_end;

tag_align __attribute__((section(".multiboot"))) multiboot_hdr_t header = {
	// Magic
	0xE85250D6,
	// Arch
	0,
	// Hdr length
	sizeof(multiboot_hdr_t),
	// Checksum (magic fields + checksum = 0)
	(uint32_t)(0-0xE85250D6-0-sizeof(multiboot_hdr_t)),

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
	multiboot_tag_t* tag = (multiboot_tag_t*)((uint8_t*)mbi+8);
	
	printk("Hello world! %s\n\r%d\n\r", "asdasd", 202);

	for (; tag->type != MULTIBOOT_TAG_TYPE_END;
	tag = (multiboot_tag_t*) ((uint8_t*) tag + ((tag->size + 7) & ~7))) {
		switch (tag->type) {
			case MULTIBOOT_TAG_TYPE_EFI_MMAP: {
				interpret_mmap((multiboot_tag_efi_mmap_t*)tag);
				break;
			}

			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
				multiboot_tag_framebuffer_t* fb_tag = (multiboot_tag_framebuffer_t*) tag;
				printk("Found FB tag! (Type %d, %dx%d)\n\r", fb_tag->common.framebuffer_type,
					fb_tag->common.framebuffer_width,fb_tag->common.framebuffer_height);
				printk("FB located at %p\n\r", fb_tag->common.framebuffer_addr);

				// First 4G is mapped, so the FB is usable
				for (uint8_t x = 0; x < 100; x++) {
					for (uint8_t y = 0; y < 100; y++) {
						*((uint32_t*) (((uint64_t) fb_tag->common.framebuffer_addr) + 4 * 1280 * y + 4 * x)) = 0x00ffffff;
					}
				}

				break;
			}
		}

		printk("New tag: %d\n\r", tag->type);
	}

	while(1);
}

void __stack_chk_fail() {}
