#pragma once

#include <types.hh>
#include <util/printf.hh>
#include <arch/limine.hh>
#include <util/async.hh>

struct framebuffer_t {
	u32* fb_addr;
	u32 fb_width;
	u32 fb_height;
	u32 fb_bpp; // bytes per pixel
};

#include <arch/amd64/amd64.hh>
using cpu_state_t = cpu_state_amd64_t;

extern struct limine_memmap_response* mmap;
extern framebuffer_t fbs[2];

extern atomic<u64> tmr_counter;

static inline void mw32(u64 a, u32 v) {
	*(volatile u32*)a = v;
}

static inline u32 mr32(u64 a) {
	return *(volatile u32*)a;
}

void arch_init(bool bsp);
void arch_late_init(bool bsp);
void arch_read_boot_info();
void arch_halt();
void arch_cli();
void arch_sti();

extern "C" [[noreturn]] void arch_cpu_state_load(cpu_state_t* state);

void sputc(const char c);
void sputs(const char* s);
char sgetc();

#define FB_VADDR 0xffffffffc2000000

extern u32* con_backbuf;
#define fb_pixel(x, y, color, idx) *(con_backbuf + (x) + ((y) * (fbs[idx].fb_width))) = color

void arch_assign_irq(u32 irq, void (*handler)(cpu_state_t* frame));
u32 arch_alloc_isr(void (*handler)(cpu_state_t* frame));
void arch_eoi();

struct timer_task {
	// Milliszekundumokban
	u64 period;
	void (*routine)();
};
template <typename T> struct vector;
extern vector<timer_task> timer_tasks;

const timer_task& arch_timer_add(const timer_task& task);
void arch_timer_remove(const timer_task& handle);
