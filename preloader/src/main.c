// VÉGRE C KÓD

#include <strings.h>
#include <serial.h>
#include <multiboot.h>
#include <memory.h>
#include <paging.h>
#include <loader.h>

void __stack_chk_fail(void) {
	printf("Stack smashing detected!\n\r");
}

void cmain(u32 mb2_header) {
	mb_tag_base_t* tag = (mb_tag_base_t*)((u64)mb2_header+8);
	u8 mmap_init = 0;

	while(tag->type) {
		if (tag->type == MB_TAG_MEMMAP) {
			mb_tag_memmap_t* mmap = (mb_tag_memmap_t*)tag;
			u8 num_entries = mmap->base.size / mmap->entry_size;

			for (u8 i = 0; i < num_entries; i++) {
				if (mmap->entries[i].type == 1) {
					// Legyen 4 GiB alatt, de a BIOS meg társai felett
					if (mmap->entries[i].addr < (u64)4*1024*1024*1024 && mmap->entries[i].addr >= 0x100000) {
						mmap_init = 1;
						bm_initialize(mmap->entries[i].addr);
					}
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
					*(u32*)(fb->fb_addr + (x*4) + (y*4*fb->fb_width)) = 0xffffffff;
				}
			}
		}

		tag = (mb_tag_base_t*)((u64)tag + ((tag->size + 7) & ~7));
	}

	if (!mmap_init) {
		printf("Nem sikerult jo helyet talalni!\n\r");
		return;
	}

	asm volatile ("movq %%cr3, %0" : "=a"(pml4));

	load_kernel();

	Elf64_Ehdr* ehdr = (Elf64_Ehdr*) &_binary_out_kernel_start;
	printf("entry: %p\n\r", ehdr->e_entry);

	// kmain lehívása
	u8 (*kmain)(void) = (u8 (*)(void)) ehdr->e_entry;

	printf("Return: %d\n\r", kmain());
}
