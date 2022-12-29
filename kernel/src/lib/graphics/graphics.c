#include "graphics.h"

void* glyph_buffer;
psf_header* def;

void clear_screen() {
    clear_screen_color(0x000000);
}

void clear_screen_color(uint32_t color) {
	for (uint32_t x = 0; x < info->g_info->info->width; x++) {
        for (uint32_t y = 0; y < info->g_info->info->height; y++) {
			pixel(x, y, color);
    	}
    }
}
