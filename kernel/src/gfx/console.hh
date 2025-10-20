#pragma once

#include <types.hh>

#define PSF1_MAGIC 0x0436
#define PSF2_MAGIC 0x864ab572

namespace console {
	pstruct psf_hdr {
		punion {
			pstruct {
				u8 magic[2];
				struct packed {
					u8 mode512 : 1;
					u8 unitable : 1;
					u8 unitable2 : 1; // ?
				} mode;
				u8 glyphsize; // bytes/glyph
			} p1;

			pstruct {
				u8 magic[4];
				u32 ver;
				u32 hdrsize;
				struct packed {
					u32 unitable : 1;
					u32 : 31;
				} flags;
				u32 numglyphs;
				u32 glyphsize; // bytes/glyph
				u32 height;
				u32 width;
			} p2;
		};
		u8 ver;
	};

	extern u8 glyphsize;
	extern u8* glyphs;
	extern u8 glyphw, glyphh;
	extern u32 cx, cy;
	extern u32 num_glyphs;
	extern u32 color_fg, color_bg;

	void init(void* psf);
	void cputc(const char c);
	void cputs(const char* s);
	void push_color(u32 color);
	void pop_color();

	void swap_buffers();
}

#include <util/printf.hh>

__attribute__((format(printf, 1, 2)))
void printk(const char* fmt, ...);
