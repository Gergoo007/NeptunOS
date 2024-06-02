#include <graphics/console.h>

u16 cx, cy;
u32 bg, fg;
font_t font;

char last_char;

void con_init(u32 _fg, u32 _bg) {
	fg = _fg;
	bg = _bg;
	fb_clear(&fb, _bg);

	psf2_hdr_t* psf2 = (psf2_hdr_t*) &FONT_START;

	font.hdr_ptr = (u64)psf2;
	font.glyphs_ptr = (u64)&FONT_START + sizeof(psf2_hdr_t);
	font.unicode_ptr = font.glyphs_ptr + psf2->glyphsize*psf2->num_glyphs;
	font.glyphsize = psf2->glyphsize;
	font.num_glyphs = psf2->num_glyphs;
	font.width = psf2->width;
	font.height = psf2->height;
}

void console_clear() {
	fb_clear(&fb, bg);
	cx = 0;
	cy = 0;
}

void update_cursor() {
	// Betűtípusban hol kezdődik a betű alulról
	u8 _height = 4;

	// Előző elrejtése
	fb_draw_rect(&fb, cx, cy + font.height - _height, font.width, 3, bg);

	// Új rajzolása
	fb_draw_rect(&fb, cx + font.width + 1, cy + font.height - _height, font.width, 3, fg);
}

void kputc(char c) {
	#ifdef PRINTK_SERIAL
		putc_translate(c);
	#endif
	if (cy >= (fb.height-font.height)) {
		console_clear();
	}

	switch (c) {
		case '\n': {
			for (u8 y = 0; y < font.height; y++)
				for (u8 x = 0; x < font.width; x++)
					fb_pixel(&fb, cx + x, cy + y, bg);

			cy += font.height + 1;
			cx = 0;

			fb_draw_rect(&fb, cx, cy + font.height - 4, font.width, 3, fg);

			return;
		}
		case '\r': {
			for (u8 y = 0; y < font.height; y++)
				for (u8 x = 0; x < font.width; x++)
					fb_pixel(&fb, cx + x, cy + y, bg);
			cx = 0;
			return;
		}
		case '\b': {
			if (cx < font.width) return;

			cx -= font.width + 1;
			for (u8 y = 0; y < font.height; y++)
				for (u8 x = 0; x < font.width*2+1; x++)
					fb_pixel(&fb, cx + x, cy + y, bg);

			fb_draw_rect(&fb, cx, cy + font.height - 4, font.width, 3, fg);

			return;
		}
	}

	if (cx + font.width*2 + 2 >= fb.width) {
		// Sortörés
		for (u8 y = 0; y < font.height; y++)
			for (u8 x = 0; x < font.width; x++)
				fb_pixel(&fb, cx + x, cy + y, bg);

		cy += font.height + 1;
		cx = 0;
	}

	u16* glyphs = (u16*) ((u64)&FONT_START + sizeof(psf2_hdr_t));
	glyphs += (c * font.height);

	update_cursor();

	for (u8 y = 0; y < font.height; y++) {
		u16 row = ((*(glyphs + y) & 0xff) << 8) | ((*(glyphs + y) & 0xff00) >> 8);
		for (u8 x = 0; x < font.width; x++) {
			if (row & ((1 << 16) >> x))
				fb_pixel(&fb, cx+x, cy+y, fg);
			else
				fb_pixel(&fb, cx+x, cy+y, bg);
		}
	}

	cx += font.width + 1;
}

void kputs(char* s) {
	while (*s) {
		kputc(*s);
		s++;
	}
}

void printk(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char* buffer = request_page();

	vsprintf(buffer, fmt, args);
	kputs(buffer);

	free_page(buffer);

	va_end(args);
}
