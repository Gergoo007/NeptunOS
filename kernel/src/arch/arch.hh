#pragma once

#include <types.hh>
#include <util/printf.hh>
#include <arch/limine.hh>

struct Machine {
	limine_memmap_response* mmap;
	struct {
		u32* fb_addr;
		u32 fb_width;
		u32 fb_height;
		u32 fb_bpp; // bytes per pixel
	} fbs[2];
};

static inline void mw32(u64 a, u32 v) {
	*(volatile u32*)a = v;
}

static inline u32 mr32(u64 a) {
	return *(volatile u32*)a;
}

namespace arch {
	void init();
	void late_init();
	void read_boot_info();
	void halt();
	void cli();
	void sti();
}

void sputc(const char c);
void sputs(const char* s);
char sgetc();

#define FB_VADDR 0xffffffffc2000000

#define fb_pixel(x, y, color, idx) *((volatile u32*)machine.fbs[idx].fb_addr + (x) + ((y) * (machine.fbs[idx].fb_width))) = color

extern Machine machine;
