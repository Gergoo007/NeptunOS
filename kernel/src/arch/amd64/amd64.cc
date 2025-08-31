#include <arch/amd64/amd64.hh>
#include <arch/amd64/idt.hh>
#include <arch/amd64/gdt.hh>
#include <arch/amd64/tss.hh>
#include <mm/vmm.hh>

#define PORT 0x3f8

static inline void outb(u16 port, u8 data) {
	asm volatile ("outb %0, %1" :: "a"((u8)data), "d"((u16)port));
}

static inline u8 inb(u16 port) {
	u8 data;
	asm volatile ("inb %%dx, %%al" : "=a"(data) : "d"((u16)port));
	return data;
}

namespace arch {
	void init() {
		sinit();
	}

	void late_init() {
		arch::gdt::init();
		arch::tss::init();
		arch::idt::init();
	}
}

void sinit() {
	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x80);
	outb(PORT + 0, 0x03);
	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x03);
	outb(PORT + 2, 0xC7);
	// outb(PORT + 4, 0x0B);
	outb(PORT + 4, 0x1E);
	outb(PORT + 0, 0xAE);

	outb(PORT + 4, 0x0F);
}

void sputc(const char c) {
	while ((inb(PORT + 5) & 0x20) == 0);
	outb(PORT, c);
}

char sgetc() {
	while ((inb(PORT + 5) & 1) == 0);
	return inb(PORT);
}
