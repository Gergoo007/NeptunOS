#include <init/multiboot.h>

void mb_parse_tags(u32 hdrp) {
	mb_tag_base_t* tag = (mb_tag_base_t*)((u64)hdrp+8);

	mb_tag_fb_t* fb_tag = NULL;
	mb_tag_memmap_t* mmap_tag = NULL;

	while (tag->type) {
		switch (tag->type) {
			case MB_TAG_FB: {
				fb_tag = (mb_tag_fb_t*)tag;
				break;
			}
			case MB_TAG_MEMMAP: {
				mmap_tag = (mb_tag_memmap_t*)tag;
				break;
			}
			case MB_TAG_KERNEL_ADDR: {
				info->preload_addr = ((mb_tag_kaddr_t*)tag)->addr;
				break;
			}
		}

		tag = (mb_tag_base_t*)((u64)tag + ((tag->size + 7) & ~7));
	}

	if (fb_tag) {
		fb_init(fb_tag->fb_addr, fb_tag->fb_width, fb_tag->fb_height, fb_tag->fb_bpp);

		con_init(0xffffffff, 0xff180101);
	}

	if (mmap_tag)
		pmm_init(mmap_tag);

	for (u64 page = 0; page < (fb_tag->fb_width*fb_tag->fb_height*fb_tag->fb_bpp/1024); page++) {
		pmm_set_used(fb_tag->fb_addr+page*PAGESIZE, 1);
	}
}
