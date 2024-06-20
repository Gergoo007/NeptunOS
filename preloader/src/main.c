// VÉGRE C KÓD

#include <strings.h>
#include <serial.h>
#include <multiboot.h>
#include <memory.h>
#include <paging.h>
#include <loader.h>

void __stack_chk_fail(void) {
	printf("Stack smashing történt!\n\r");
}

mb_tag_fb_t* fb = NULL;

extern void* PRELOADER_START;
extern void* PRELOADER_END;

kernel_info_t kinfo;

void cmain(u32 mb2_header) {
	mb_tag_base_t* tag = (mb_tag_base_t*)((u64)mb2_header+8);
	u8 mmap_init = 0;

	while(tag->type) {
		if (tag->type == MB_TAG_MEMMAP) {
			mb_tag_memmap_t* mmap = (mb_tag_memmap_t*)tag;
			u8 num_entries = mmap->base.size / mmap->entry_size;

			for (u8 i = 0; i < num_entries; i++) {
				if (mmap->entries[i].type == 1) {
					if (mmap->entries[i].addr >= 0x100000 && mmap->entries[i].addr < 4ULL*1024*1024*1024 && mmap->entries[i].length >= PAGESIZE*20) {
						if (!mmap_init) {
							mmap_init = 1;
							bm_initialize(mmap->entries[i].addr);
						}
					}
				}
			}
		} else if (tag->type == MB_TAG_FB) {
			fb = (mb_tag_fb_t*)tag;
			if (fb->fb_addr > (u64)4*1024*1024*1024) {
				printf("A framebuffer a 4 GB-s határ felett van!\n\r");
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
		printf("Nem sikerült jó helyet találni!\n\r");
		for (u8 x = 0; x < 200; x++) {
			for (u8 y = 0; y < 200; y++) {
				*(u32*)(fb->fb_addr + (x*4) + (y*4*fb->fb_width)) = 0xffff0000;
			}
		}
		return;
	}

	// PML4 betöltése
	asm volatile ("movq %%cr3, %0" : "=a"(pml4));

	kinfo.mb_hdr_addr = mb2_header;
	kinfo.preload_end = (u64)&PRELOADER_END;

	u64 entry = load_kernel();
	u8 (*kmain)(kernel_info_t*) = (u8 (*)(kernel_info_t*)) entry;
	printf("Return: %d\n\r", kmain(&kinfo));

	printf("Preloader elérte a program végét?!\n\r");
}
