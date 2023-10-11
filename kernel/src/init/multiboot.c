#include <init/multiboot.h>

void mb_parse_tags(u32 hdrp) {
	mb_tag_base_t* tag = (mb_tag_base_t*)((u64)hdrp+8);

	while (tag->type) {
		sprintk("Type: %d, size: %d\n\r", tag->type, tag->size);

		switch (tag->type) {
			case MB_TAG_FB: {
				mb_tag_fb_t* fb_tag = (mb_tag_fb_t*)tag;
				fb_init(fb_tag->fb_addr, fb_tag->fb_width, fb_tag->fb_height, fb_tag->fb_bpp);
				fb_draw_rect(&fb, 500, 500, 100, 100, 0x00ff0000);
				// for (u8 x = 0; x < 200; x++) {
				// 	for (u8 y = 0; y < 200; y++) {
				// 		*(u32*)(fb.base + (x*4) + (y*4*fb.width)) = 0xffff0000;
				// 	}
				// }
				break;
			}
		}

		tag = (mb_tag_base_t*)((u64)tag + ((tag->size + 7) & ~7));
	}
}
