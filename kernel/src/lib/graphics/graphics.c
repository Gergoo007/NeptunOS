#include "graphics.h"

void clear_screen() {
    clear_screen_color(0x000000);
}

void clear_screen_color(uint32_t color) {
	for (uint32_t x = 0; x < graphics->framebuffer->Width; x++) {
        for (uint32_t y = 0; y < graphics->framebuffer->Height; y++) {
			pixel(x, y, color);
    	}
    }
}
