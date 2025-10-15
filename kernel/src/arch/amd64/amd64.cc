#include <arch/amd64/amd64.hh>
#include <arch/amd64/idt.hh>
#include <arch/amd64/gdt.hh>
#include <arch/amd64/tss.hh>
#include <arch/amd64/io.hh>
#include <mm/vmm.hh>

#define PORT 0x3f8

extern "C" void sse_init();

namespace arch {
	void init() {
		sse_init();
		sinit();
	}

	void late_init() {
		arch::gdt::init();
		arch::tss::init();
		arch::idt::init();
	}

	void halt() {
		asm volatile ("hlt");
	}
}

void sinit() {
	arch::outb(PORT + 1, 0x00);
	arch::outb(PORT + 3, 0x80);
	arch::outb(PORT + 0, 0x03);
	arch::outb(PORT + 1, 0x00);
	arch::outb(PORT + 3, 0x03);
	arch::outb(PORT + 2, 0xC7);
	// outb(PORT + 4, 0x0B);
	arch::outb(PORT + 4, 0x1E);
	arch::outb(PORT + 0, 0xAE);

	arch::outb(PORT + 4, 0x0F);
}

void sputc(const char c) {
	while ((arch::inb(PORT + 5) & 0x20) == 0);
	arch::outb(PORT, c);
	if (c == '\n')
		sputc('\r');
}

char sgetc() {
	while ((arch::inb(PORT + 5) & 1) == 0);
	return arch::inb(PORT);
}

