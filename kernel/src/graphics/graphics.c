#include <graphics/graphics.h>

framebuffer_t fb;

// Beállít egy framebuffer structot a releváns
// információkkal
void fb_init(u64 base, u32 width, u32 height, u8 bpp) {
	fb.base = base;
	fb.width = width;
	fb.height = height;
	fb.bpp = bpp;
}

void fb_draw_rect(framebuffer_t* _fb, u32 x, u32 y, u32 width, u32 height, u32 color) {
	// Csak úgy működik ha balról jobbra írjuk ki a pixeleket
	for (u32 i = 0; i < width; i++) {
		for (u32 j = 0; j < height; j++) {
			*(u32*)(_fb->base + ((i+x)*(_fb->bpp/8)) + ((j+y)*(_fb->bpp/8)*_fb->width)) = color;
		}
	}
}

void fb_clear(framebuffer_t* _fb, u32 color) {
	fb_draw_rect(_fb, 0, 0, _fb->width, _fb->height, color);
}

void fb_pixel(framebuffer_t* _fb, u32 x, u32 y, u32 color) {
	if (_fb->bpp != 32) {
		sprintk("A BPP érték még nem támogatott!\n\r");
		return;
	}

	// *(u32*)((u64)fb->base + x*(fb->bpp/8) + y*(fb->bpp/8*fb->width)) = color;
	*(u32*)(_fb->base + (x*4) + (y*4*_fb->width)) = color;
}
