#include <graphics/console.h>

u16 cx, cy;
u32 bg, fg;
font_t font;

void con_init(u32 _fg, u32 _bg) {
	fg = _fg;
	bg = _bg;
	fb_draw_rect(&fb, 0, 0, fb.width, fb.height, _bg);

	psf2_hdr_t* psf2 = (psf2_hdr_t*) &FONT_START;

	font.hdr_ptr = (u64)psf2;
	font.glyphs_ptr = (u64)&FONT_START + sizeof(psf2_hdr_t);
	font.unicode_ptr = font.glyphs_ptr + psf2->glyphsize*psf2->num_glyphs;
	font.glyphsize = psf2->glyphsize;
	font.num_glyphs = psf2->num_glyphs;
	font.width = psf2->width;
	font.height = psf2->height;
}

void kputc(char c) {
	if (c == '\n') {
		cy += font.height + 1;
		cx = 0;
		return;
	} else if (c == '\r') {
		cx = 0;
		return;
	}

	u16* glyphs = (u16*) ((u64)&FONT_START + sizeof(psf2_hdr_t));
	glyphs += (c * font.height);

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

	#ifdef PRINTK_SERIAL
	puts_translate(buffer);
	#endif

	free_page(buffer);

	va_end(args);
}
