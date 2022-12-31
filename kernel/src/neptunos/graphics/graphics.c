#include <neptunos/graphics/graphics.h>
#include <neptunos/libk/string.h>

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

#ifdef USE_DOUBLE_BUFFERING
	void* back_buffer;
	void setup_back_buffer() {
		back_buffer = malloc(info->g_info->fb_size);
		memcpy(back_buffer, info->g_info->fb_base, info->g_info->fb_size);
	}

	void sync_back_buffer() {
		memcpy(info->g_info->fb_base, back_buffer, info->g_info->fb_size);
	}
#endif
