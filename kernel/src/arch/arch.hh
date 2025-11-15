#pragma once

#include <types.hh>
#include <util/printf.hh>
#include <arch/limine.hh>

struct framebuffer_t {
	u32* fb_addr;
	u32 fb_width;
	u32 fb_height;
	u32 fb_bpp; // bytes per pixel
};

extern struct limine_memmap_response* mmap;
extern framebuffer_t fbs[2];

static inline void mw32(u64 a, u32 v) {
	*(volatile u32*)a = v;
}

static inline u32 mr32(u64 a) {
	return *(volatile u32*)a;
}

void arch_init();
void arch_late_init();
void arch_read_boot_info();
void arch_halt();
void arch_cli();
void arch_sti();

void sputc(const char c);
void sputs(const char* s);
char sgetc();

#define FB_VADDR 0xffffffffc2000000

extern u32* con_backbuf;
#define fb_pixel(x, y, color, idx) *(con_backbuf + (x) + ((y) * (fbs[idx].fb_width))) = color
