#pragma once

#include <lib/types.h>
#include <serial/serial.h>

#pragma pack(1)

// BPP mint BITS / pixel
typedef struct framebuffer {
	u64 base;
	u32 width;
	u32 height;
	u8 bpp;
} framebuffer;

extern framebuffer fb;

void fb_init(u64 base, u32 width, u32 height, u8 bpp);
void fb_pixel(framebuffer* fb, u32 x, u32 y, u32 color);
void fb_draw_rect(framebuffer* fb, u32 x, u32 y, u32 width, u32 height, u32 color);
void fb_clear(framebuffer* fb, u32 color);
