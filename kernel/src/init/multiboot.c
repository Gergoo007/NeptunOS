#include <init/multiboot.h>

void mb_parse_tags(u32 hdrp) {
	mb_tag_base_t* tag = (mb_tag_base_t*)((u64)hdrp+8);

	while (tag->type) {
		switch (tag->type) {
			case MB_TAG_FB: {
				mb_tag_fb_t* fb_tag = (mb_tag_fb_t*)tag;
				fb_init(fb_tag->fb_addr, fb_tag->fb_width, fb_tag->fb_height, fb_tag->fb_bpp);
				break;
			}
		}

		tag = (mb_tag_base_t*)((u64)tag + ((tag->size + 7) & ~7));
	}
}
