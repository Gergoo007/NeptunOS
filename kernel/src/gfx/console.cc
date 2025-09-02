#include <gfx/console.hh>
#include <util/mem.hh>
#include <arch/arch.hh>

namespace console {
	u8 glyphsize;
	u8* glyphs;
	u8 glyphw, glyphh;
	u32 num_glyphs;

	u32 cx = 0, cy = 0;
	u32 padx = 1, pady = 1;

	u32 color_fg = 0xffd0d0d0, color_bg = 0xff000000;

	u32 current_fb = 0;

	void init(void* psf) {
		psf_hdr* p = (psf_hdr*)psf;

		if (*(u16*)psf == PSF1_MAGIC) {
			glyphs = (u8*) ((u64)psf + sizeof(p->p1));
			glyphsize = p->p1.glyphsize;
			glyphw = 8;
			glyphh = glyphsize;
			num_glyphs = p->p1.mode.mode512 ? 512 : 256;
			sprintk("PSF1 betűtípus");
		} else if (*(u32*)psf == PSF2_MAGIC) {
			glyphs = (u8*) ((u64)psf + p->p2.hdrsize);
			glyphsize = p->p2.glyphsize;
			glyphw = p->p2.width;
			glyphh = p->p2.height;
			num_glyphs = p->p2.numglyphs;
			sprintk("PSF2 betűtípus");
		} else {
			sputs("Ismeretlen betűtípus\n\r");
		}

		if (glyphw > 16) sprintk("Túl nagy a betűtípus!\n\r");
		sprintk(" %dx%d (%d x %d glyphs)\n\r", glyphw, glyphh, num_glyphs, glyphsize);
	}

	void cputc(const char c) {
		switch (c) {
			case '\n': {
				cy += glyphh + pady;
				cx = 0;
				return;
			}
			default: break;
		}

		if (cx + glyphw + padx > machine.fbs[current_fb].fb_width) {
			cputc('\n');
		}

		u8* start = (u8*)(glyphs + c * glyphsize);

		for (u32 y = 0; y < glyphh; y++) {
			u16 row = 0;
			if (glyphw > 8) {
				row = *(u16*)start;
				row = ((row & 0xff) << 8) | ((row & 0xff00) >> 8);
				start += 2;
			} else {
				row = *(u8*)start++;
			}

			for (u32 x = 0; x < glyphw; x++)
				fb_pixel(cx + x, cy + y, row & (1 << ((glyphw > 8 ? 16 : 8) - x)) ? color_fg : color_bg, current_fb);
		}

		cx += glyphw + padx;
	}

	void cputs(const char* s) {
		while (*s) {
			cputc(*(s++));
		}
	}

	u32 old_color = 0;

	void push_color(u32 color) {
		old_color = color_fg;
		color_fg = color;
	}

	void pop_color() {
		color_fg = old_color;
	}
}

__attribute__((format(printf, 1, 2)))
void printk(const char* fmt, ...) {
	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);
}
