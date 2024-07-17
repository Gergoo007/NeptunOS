#include <gfx/framebuffer.h>

void fb_draw_rect(framebuffer* fb, u32 x, u32 y, u32 w, u32 h, u32 c) {
	for (u32 i = 0; i < w; i++) {
		for (u32 j = 0; j < h; j++) {
			// *(u32*)(fb.base + ((i+x)*(4)) + ((j+y)*(4)*fb.width)) = c;
			fb_pixel((*fb), x+i, y+j, c);
		}
	}
}
