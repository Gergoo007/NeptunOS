#pragma once

#include <lib/int.h>
#include <serial/serial.h>

#pragma pack(1)

// BPP mint BITS / pixel
typedef struct framebuffer {
	u64 base;
	u32 width;
	u32 height;
	u8 bpp;
} framebuffer_t;

extern framebuffer_t fb;

void fb_init(u64 base, u32 width, u32 height, u8 bpp);
void fb_pixel(framebuffer_t* fb, u32 x, u32 y, u32 color);
void fb_draw_rect(framebuffer_t* fb, u32 x, u32 y, u32 width, u32 height, u32 color);
void fb_clear(framebuffer_t* fb, u32 color);
