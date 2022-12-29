#pragma once

#include <stdint.h>

#include "../globals.h"

typedef struct psf2_header {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
} __attribute__((packed)) psf2_header;

typedef struct {
	uint8_t magic1;
	uint8_t magic2;
	uint8_t mode;
	uint8_t bytesperglyph;
} __attribute__((packed)) psf1_header;

// Universal psf header used in the kernel
// if the font is psf1, it will get copied into this struct,
// if it's psf2, then it will get memcpy'd into this struct as it is
typedef struct psf_header {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
} __attribute__((packed)) psf_header;

extern psf_header* def;
extern void* glyph_buffer;

static inline void pixel(uint32_t x, uint32_t y, uint32_t color) {
   /**((uint32_t*) ((uint64_t)info->g_info->fb_base + (x * sizeof(uint32_t)) + (y * info->g_info->info->scanline))) = color;*/
   *((uint32_t*) (((uint64_t) info->g_info->fb_base) + 4 * info->g_info->info->width * y + 4 * x)) = color;
}

void clear_screen(void);
void clear_screen_color(uint32_t color);
