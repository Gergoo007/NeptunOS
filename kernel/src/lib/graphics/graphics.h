#pragma once

#include <stdint.h>

#include "../globals.h"

static inline void pixel(uint32_t x, uint32_t y, uint32_t color) {
    *((uint32_t*) (((uint64_t) graphics->framebuffer->BaseAddress) + 4 * graphics->framebuffer->PixelsPerScanLine * y + 4 * x)) = color;
}

void clear_screen(void);
void clear_screen_color(uint32_t color);
