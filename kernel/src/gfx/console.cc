#include <gfx/console.hh>
#include <util/mem.hh>
#include <util/stacktrace.hh>
#include <arch/arch.hh>
#include <mm/vmm.hh>

u8 con_glyphsize;
u8* con_glyphs;
u8 con_glyphw, con_glyphh;
u32 con_num_glyphs;

u32 con_cx = 0, con_cy = 0;
u32 con_padx = 1, con_pady = 1;

u32 con_color_fg = 0xffd0d0d0, con_color_bg = 0xff000000;

u32 current_fb = 0;

u8 con_inited = false;

u32* con_backbuf = (u32*)0x6161616161616161;
u64 con_backbuf_size;

constexpr u32 tab_width = 8;

void con_init(void* psf) {
	con_inited = true;
	psf_hdr_t* p = (psf_hdr_t*)psf;

	if (*(u16*)psf == PSF1_MAGIC) {
		con_glyphs = (u8*) ((u64)psf + sizeof(p->p1));
		con_glyphsize = p->p1.glyphsize;
		con_glyphw = 8;
		con_glyphh = con_glyphsize;
		con_num_glyphs = p->p1.mode.mode512 ? 512 : 256;
		sprintk("PSF1 betűtípus");
	} else if (*(u32*)psf == PSF2_MAGIC) {
		con_glyphs = (u8*) ((u64)psf + p->p2.hdrsize);
		con_glyphsize = p->p2.glyphsize;
		con_glyphw = p->p2.width;
		con_glyphh = p->p2.height;
		con_num_glyphs = p->p2.numglyphs;
		sprintk("PSF2 betűtípus");
	} else {
		sputs("Ismeretlen betűtípus\n\r");
	}

	if (con_glyphw > 16) sprintk("Túl nagy a betűtípus!\n\r");
	sprintk(" %dx%d (%d x %d glyphs)\n\r", con_glyphw, con_glyphh, con_num_glyphs, con_glyphsize);

	con_backbuf_size =
		fbs[current_fb].fb_width *
		fbs[current_fb].fb_height *
		(fbs[current_fb].fb_bpp / 8);
	// cache line-ra turi ippelni ajánlott
	con_backbuf = (u32*)kmalloc_aligned(con_backbuf_size, 64);
	memset(con_backbuf, 0, con_backbuf_size);
	sprintk("Backbuffer @ %p of size %llx\n\r", con_backbuf, con_backbuf_size);
}

// backbuf-t görgeti, nem swappol
void con_scroll() {
	const auto* fb = &fbs[current_fb];
	u32* fb_base = con_backbuf;
	u32 lineh = con_glyphh + con_pady;

	memcpy(fb_base, fb_base + fb->fb_width * lineh, fb->fb_width * (fb->fb_height - lineh) * (fb->fb_bpp/8));

	con_cy -= lineh;

	memset(fb_base + (fb->fb_width * (fb->fb_height - lineh)), 0, fb->fb_width * lineh * (fb->fb_bpp/8));

	vmm_check(con_backbuf);
}

void con_swap_buffers() {
	// memcpy(machine.fbs[current_fb].fb_addr, backbuf, backbuf_size);
	u32* fb_base = fbs[current_fb].fb_addr;

	#ifdef __x86_64__
	if ((con_backbuf_size & 31) == 0) {
		// AVX move, 32 byte egyszerre
		for (u32 i = 0; i < con_backbuf_size / 32; i++) {
			asm volatile ("prefetchnta (%0)" :: "r"((u64)con_backbuf + i * 32 + 256));
			asm volatile (
				"vmovdqu (%0), %%ymm0\n"
				"vmovntdq %%ymm0, (%1)\n" ::
				"r"((u64)con_backbuf + i * 32), "r"((u64)fb_base + i * 32) :
				"ymm0", "memory"
			);
		}
		asm volatile ("sfence");
		return;
	}
	#endif

	if (con_backbuf_size & 7)
		for (u32 i = 0; i < con_backbuf_size / 4; i++)
			fbs[current_fb].fb_addr[i] = con_backbuf[i];
	else
		for (u32 i = 0; i < con_backbuf_size / 8; i++)
			((u64*)(fbs[current_fb].fb_addr))[i] = ((u64*)con_backbuf)[i];
}

void cputc(const char c) {
	if (!con_inited) {
		sprintk("yo no console::init yet");
		pause();
	}

	switch (c) {
		case '\n': {
			con_cy += con_glyphh + con_pady;
			con_cx = 0;

			if (con_cy >= fbs[current_fb].fb_height - con_glyphh)
				con_scroll();

			return;
		}
		case '\t': {
			u32 cw = con_glyphw + con_padx;
			con_cx += cw;
			con_cx = align(con_cx, cw * tab_width);
			if (con_cx + con_glyphw + con_padx > fbs[current_fb].fb_width)
				cputc('\n');
			return;
		}
		case '\r': {
			con_cx = 0;
			return;
		}
		default: break;
	}

	if (con_cx + con_glyphw + con_padx > fbs[current_fb].fb_width)
		cputc('\n');

	u8* start = (u8*)(con_glyphs + c * con_glyphsize);

	for (u32 y = 0; y < con_glyphh; y++) {
		u16 row = 0;
		if (con_glyphw > 8) {
			row = *(u16*)start;
			// row = ((row & 0xff) << 8) | ((row & 0xff00) >> 8);
			row = __builtin_bswap16(row);
			start += 2;
		} else {
			row = *(u8*)start++;
		}

		u16 mask = 1 << (con_glyphw > 8 ? 16 : 8);
		for (u32 x = 0; x < con_glyphw; x++) {
			fb_pixel(con_cx + x, con_cy + y, (row & mask) ? con_color_fg : con_color_bg, current_fb);
			mask >>= 1;
		}
	}

	con_cx += con_glyphw + con_padx;
}

void cputs(const char* s) {
	while (*s) {
		cputc(*(s++));
	}
	con_swap_buffers();
}

u32 con_old_color = 0;

void con_push_color(u32 color) {
	con_old_color = con_color_fg;
	con_color_fg = color;
}

void con_pop_color() {
	con_color_fg = con_old_color;
}

void con_clear() {
	con_cx = con_cy = 0;
	memset(con_backbuf, 0x00, con_backbuf_size);
	memset(fbs[current_fb].fb_addr, 0x00, con_backbuf_size);
}

__attribute__((format(printf, 1, 2)))
void printk(const char* fmt, ...) {
	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);

	con_swap_buffers();

	#ifdef SERIALPRINTK
	va_list list2;
	va_start(list2, fmt);
	vprintf2(fmt, list2);
	va_end(list2);
	#endif
}

__attribute__((format(printf, 3, 4)))
void printkx(u32 lvl, const char* FILENAME, const char* fmt, ...) {
	// Debug?
	// Ha nincs a fájl a debug files listába, ne is printeljen
	if (!cfg_willitprint(lvl, FILENAME)) return;

	con_push_color(con_colors[lvl]);

	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);

	con_swap_buffers();

	#ifdef SERIALPRINTK
	va_list list2;
	va_start(list2, fmt);
	vprintf2(fmt, list2);
	va_end(list2);
	#endif

	con_pop_color();
}

__attribute__((format(printf, 3, 4)))
void printkxnoret(u32 lvl, const char* FILENAME, const char* fmt, ...) {
	if (!cfg_willitprint(lvl, FILENAME)) pause();

	con_push_color(con_colors[lvl]);

	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);

	con_swap_buffers();

	#ifdef SERIALPRINTK
	va_list list2;
	va_start(list2, fmt);
	vprintf2(fmt, list2);
	va_end(list2);
	#endif

	if constexpr (STACKTRACE_ON_FATAL)
		stacktrace();
	pause();

	con_pop_color();
}

__attribute__((format(printf, 1, 2)))
void sprintk(const char* fmt, ...) {
	va_list list;
	va_start(list, fmt);
	vprintf2(fmt, list);
	va_end(list);
}
