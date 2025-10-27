#include <gfx/console.hh>
#include <util/mem.hh>
#include <arch/arch.hh>
#include <mm/vmm.hh>
#include <util/cpuid.hh>

namespace console {
	u8 glyphsize;
	u8* glyphs;
	u8 glyphw, glyphh;
	u32 num_glyphs;

	u32 cx = 0, cy = 0;
	u32 padx = 1, pady = 1;

	u32 color_fg = 0xffd0d0d0, color_bg = 0xff000000;

	u32 current_fb = 0;

	u8 inited = false;

	u32* backbuf = (u32*)0x6161616161616161;
	u64 backbuf_size;

	void init(void* psf) {
		inited = true;
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

		backbuf_size =
			machine.fbs[current_fb].fb_width *
			machine.fbs[current_fb].fb_height *
			(machine.fbs[current_fb].fb_bpp / 8);
		// cache line-ra turi ippelni ajánlott
		backbuf = (u32*)kmalloc_aligned(backbuf_size, 64);
		memset(backbuf, 0, backbuf_size);
		sprintk("Backbuffer @ %p of size %llx\n\r", backbuf, backbuf_size);
	}

	// backbuf-t görgeti, nem swappol
	void scroll() {
		const auto& fb = machine.fbs[current_fb];
		u32* fb_base = backbuf;
		u32 lineh = glyphh + pady;

		memcpy(fb_base, fb_base + fb.fb_width * lineh, fb.fb_width * (fb.fb_height - lineh) * (fb.fb_bpp/8));

		cy -= lineh;

		memset(fb_base + (fb.fb_width * (fb.fb_height - lineh)), 0, fb.fb_width * lineh * (fb.fb_bpp/8));
	}

	void swap_buffers() {
		// memcpy(machine.fbs[current_fb].fb_addr, backbuf, backbuf_size);
		u32* fb_base = machine.fbs[current_fb].fb_addr;

		#ifdef __x86_64__
		if ((backbuf_size & 31) == 0) {
			// AVX move, 32 byte egyszerre
			for (u32 i = 0; i < backbuf_size / 32; i++) {
				asm volatile ("prefetchnta (%0)" :: "r"((u64)backbuf + i * 32 + 256));
				asm volatile (
					"vmovdqu (%0), %%ymm0\n"
					"vmovntdq %%ymm0, (%1)" ::
					"r"((u64)backbuf + i * 32), "r"((u64)fb_base + i * 32) :
					"ymm0", "memory"
				);
			}
			asm volatile ("sfence");
			return;
		}
		#endif

		if (backbuf_size & 7)
			for (u32 i = 0; i < backbuf_size / 4; i++)
				machine.fbs[current_fb].fb_addr[i] = backbuf[i];
		else
			for (u32 i = 0; i < backbuf_size / 8; i++)
				((u64*)(machine.fbs[current_fb].fb_addr))[i] = ((u64*)backbuf)[i];
	}

	void cputc(const char c) {
		if (!inited) {
			sprintk("yo no console::init yet\n");
			pause();
		}

		switch (c) {
			case '\n': {
				cy += glyphh + pady;
				cx = 0;

				if (cy >= machine.fbs[current_fb].fb_height - glyphh)
					scroll();

				return;
			}
			case '\r': {
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
		swap_buffers();
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

	console::swap_buffers();

	#ifdef SERIALPRINTK
	va_list list2;
	va_start(list2, fmt);
	vprintf2(fmt, list2);
	va_end(list2);
	#endif
}

__attribute__((format(printf, 1, 2)))
void sprintk(const char* fmt, ...) {
	va_list list;
	va_start(list, fmt);
	vprintf2(fmt, list);
	va_end(list);
}
