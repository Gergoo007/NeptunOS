#pragma once

#include <util/types.h>
#include <util/attrs.h>

typedef struct _attr_packed framebuffer {
	u64 base;
	u32 size;
	u32 width;
	u32 height;
} framebuffer;

extern framebuffer fb_main;

// #define fb_pixel(fb, x, y, color) *(u32*)((u64)fb.base + x*4 + (y * fb.width * 4)) = color
#define fb_pixel(fb, x, y, color) *((u32*)fb.base + (x) + ((y) * fb.width)) = color

void fb_draw_rect(framebuffer* fb, u32 x, u32 y, u32 w, u32 h, u32 c);
