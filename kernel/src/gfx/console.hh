#pragma once

#include <types.hh>

#define PSF1_MAGIC 0x0436
#define PSF2_MAGIC 0x864ab572

typedef pstruct psf_hdr {
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
} psf_hdr_t;

extern u8 con_glyphsize;
extern u8* con_glyphs;
extern u8 con_glyphw, con_glyphh;
extern u32 con_cx, con_cy;
extern u32 con_num_glyphs;
extern u32 con_color_fg, con_color_bg;
extern u8 con_inited;

void con_init(void* psf);
void con_cputc(const char c);
void con_cputs(const char* s);
void con_push_color(u32 color);
void con_pop_color();
void con_clear();

void con_swap_buffers();

#include <util/printf.hh>

__attribute__((format(printf, 1, 2)))
void printk(const char* fmt, ...);


static constexpr u32 con_colors[5] = {
	0xffa0a0a0,
	0xffd0d0d0,
	0xffEB6534,
	0xffC41E3D,
	0xff710627,
};
