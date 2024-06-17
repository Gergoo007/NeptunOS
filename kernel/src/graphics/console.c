#include <graphics/console.h>

u16 con_cx, con_cy;
u32 con_bg, con_fg;
font con_font;

char last_char;

void con_init(u32 _fg, u32 _bg) {
	con_fg = _fg;
	con_bg = _bg;
	fb_clear(&fb, _bg);

	psf2_hdr* psf2 = (psf2_hdr*) &FONT_START;

	con_font.hdr_ptr = (u64)psf2;
	con_font.glyphs_ptr = (u64)&FONT_START + sizeof(psf2_hdr);
	con_font.unicode_ptr = con_font.glyphs_ptr + psf2->glyphsize*psf2->num_glyphs;
	con_font.glyphsize = psf2->glyphsize;
	con_font.num_glyphs = psf2->num_glyphs;
	con_font.width = psf2->width;
	con_font.height = psf2->height;
}

void console_clear() {
	fb_clear(&fb, con_bg);
	con_cx = 0;
	con_cy = 0;
}

void update_cursor() {
	// Betűtípusban hol kezdődik a betű alulról
	u8 _height = 4;

	// Előző elrejtése
	fb_draw_rect(&fb, con_cx, con_cy + con_font.height - _height, con_font.width, 3, con_bg);

	// Új rajzolása
	fb_draw_rect(&fb, con_cx + con_font.width + 1, con_cy + con_font.height - _height, con_font.width, 3, con_fg);
}

void kputc(char c) {
	#ifdef PRINTK_SERIAL
		putc_translate(c);
	#endif
	if (con_cy >= (fb.height-con_font.height)) {
		console_clear();
	}

	switch (c) {
		case '\n': {
			for (u8 y = 0; y < con_font.height; y++)
				for (u8 x = 0; x < con_font.width; x++)
					fb_pixel(&fb, con_cx + x, con_cy + y, con_bg);

			con_cy += con_font.height + 1;
			con_cx = 0;

			fb_draw_rect(&fb, con_cx, con_cy + con_font.height - 4, con_font.width, 3, con_fg);

			return;
		}
		case '\r': {
			for (u8 y = 0; y < con_font.height; y++)
				for (u8 x = 0; x < con_font.width; x++)
					fb_pixel(&fb, con_cx + x, con_cy + y, con_bg);
			con_cx = 0;
			return;
		}
		case '\b': {
			if (con_cx < con_font.width) return;

			con_cx -= con_font.width + 1;
			for (u8 y = 0; y < con_font.height; y++)
				for (u8 x = 0; x < con_font.width*2+1; x++)
					fb_pixel(&fb, con_cx + x, con_cy + y, con_bg);

			fb_draw_rect(&fb, con_cx, con_cy + con_font.height - 4, con_font.width, 3, con_fg);

			return;
		}
	}

	if ((u32)con_cx + con_font.width*2 + 2 >= fb.width) {
		// Sortörés
		for (u8 y = 0; y < con_font.height; y++)
			for (u8 x = 0; x < con_font.width; x++)
				fb_pixel(&fb, con_cx + x, con_cy + y, con_bg);

		con_cy += con_font.height + 1;
		con_cx = 0;
	}

	u16* glyphs = (u16*) ((u64)&FONT_START + sizeof(psf2_hdr));
	glyphs += (c * con_font.height);

	update_cursor();

	for (u8 y = 0; y < con_font.height; y++) {
		u16 row = ((*(glyphs + y) & 0xff) << 8) | ((*(glyphs + y) & 0xff00) >> 8);
		for (u8 x = 0; x < con_font.width; x++) {
			if (row & ((1 << 16) >> x))
				fb_pixel(&fb, con_cx+x, con_cy+y, con_fg);
			else
				fb_pixel(&fb, con_cx+x, con_cy+y, con_bg);
		}
	}

	con_cx += con_font.width + 1;
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

void _report(const char* fmt, const char* file, ...) {
	// Ha benne van a _logs-ban csak akkor printeljük
	if (!sizeof(_logs)) return;
	for (u8 i = 0; i < sizeof(_logs) / sizeof(*_logs); i++) {
		if (!strcmp(file, _logs[i])) break;
		if (i == sizeof(_logs) / sizeof(*_logs) - 1) return;
	}

	u32 old = con_fg;
	con_fg = 0x00fff000;

	va_list args;
	va_start(args, file);

	char* buffer = request_page();

	vsprintf(buffer, fmt, args);
	kputc('[');
	kputs((char*)file);
	kputc(']');
	kputc(' ');
	kputs(buffer);

	free_page(buffer);

	va_end(args);
	con_fg = old;

	kputc('\n');
}

void _warn(const char* fmt, const char* file, ...) {
	u32 old = con_fg;
	con_fg = 0xff6700;

	va_list args;
	va_start(args, file);

	char* buffer = request_page();

	vsprintf(buffer, fmt, args);
	kputc('[');
	kputs((char*)file);
	kputc(']');
	kputc(' ');
	kputs(buffer);

	free_page(buffer);

	va_end(args);
	con_fg = old;

	kputc('\n');
}

void _error(const char* fmt, const char* file, ...) {
	u32 old = con_fg;
	con_fg = 0x00ff0000;

	va_list args;
	va_start(args, file);

	char* buffer = request_page();

	vsprintf(buffer, fmt, args);
	kputc('[');
	kputs((char*)file);
	kputc(']');
	kputc(' ');
	kputs(buffer);

	free_page(buffer);

	va_end(args);
	con_fg = old;

	kputc('\n');
}
