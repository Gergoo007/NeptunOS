// VÉGRE C KÓD

#include <strings.h>
#include <serial.h>
#include <multiboot.h>

void __stack_chk_fail(void) {
	printf("Stack smashing detected!\n\r");
}

void cmain(u32 mb2_header) {
	mb_tag_base_t* tag = (mb_tag_base_t*)((u64)mb2_header+8);
	u32 total_ram = 0;

	while(tag->type) {
		printf("Tag type: %d, size: %d\n\r", tag->type, tag->size);

		if (tag->type == MB_TAG_MEMMAP) {
			mb_tag_memmap_t* mmap = (mb_tag_memmap_t*)tag;
			u8 num_entries = mmap->base.size / mmap->entry_size;

			for (u8 i = 0; i < num_entries; i++) {
				switch (mmap->entries[i].type) {
					// Ezek a típusok biztosan léteznek
					// fizikailag, de a többi "reserved"
					// valószínűleg csak MMIO
					case 1:
					case 3:
					case 4:
					case 5:
						total_ram += mmap->entries[i].length;
						break;
					default:
						break;
				}
			}
		} else if (tag->type == MB_TAG_FB) {
			mb_tag_fb_t* fb = (mb_tag_fb_t*)tag;
			if (fb->fb_addr > (u64)4*1024*1024*1024) {
				printf("The framebuffer can't be used because it's above the mapping!\n\r");
				continue;
			}

			for (u8 x = 0; x < 200; x++) {
				for (u8 y = 0; y < 200; y++) {
					u64 pixel = fb->fb_addr + (x*4) + (y*4*fb->fb_width);
					if (pixel > ((u64)4*1024*1024*1024)) {
						printf("Ajajj\n\r");
						asm volatile("cli");
						asm volatile("hlt");
					}
					*(u32*)(fb->fb_addr + (x*4) + (y*4*fb->fb_width)) = 0xffffffff;
				}
			}
		}

		tag = (mb_tag_base_t*)((u64)tag + ((tag->size + 7) & ~7));
	}

	printf("Fun fact: osszesen %d MiB RAM all rendelkezesre.\n\r", total_ram / 1024 / 1024);
}
