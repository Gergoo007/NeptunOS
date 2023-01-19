#pragma once

#include <neptunos/libk/stdint.h>

#include <neptunos/globals.h>

extern void* fb_base;

typedef struct psf2_header_t {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
} _attr_packed psf2_header_t;

typedef struct psf1_header_t {
	uint8_t magic1;
	uint8_t magic2;
	uint8_t mode;
	uint8_t bytesperglyph;
} _attr_packed psf1_header_t;

// Universal psf header used in the kernel
// if the font is psf1, it will get copied into this struct,
// if it's psf2, then it will get memcpy'd into this struct as it is
typedef struct psf_header_t {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
} _attr_packed psf_header_t;

extern psf_header_t* def;
extern void* glyph_buffer;
extern void* back_buffer;

#define pixel(x, y, color) *((uint32_t*) (((uint64_t) fb_base) + 4 * info->g_info->info->width * y + 4 * x)) = color;\

void clear_screen(void);
void clear_screen_color(uint32_t color);

void setup_back_buffer();
void sync_back_buffer();
