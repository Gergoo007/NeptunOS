#include <arch/amd64/amd64.hh>
#include <arch/amd64/idt.hh>
#include <arch/amd64/gdt.hh>
#include <arch/amd64/tss.hh>
#include <arch/amd64/io.hh>
#include <mm/vmm.hh>

#define PORT 0x3f8

extern "C" void sse_init();

void arch_init() {
	sse_init();
	sinit();
}

void arch_late_init() {
	arch_gdt_init();
	arch_tss_init();
	arch_idt_init();
}

void arch_halt() {
	asm volatile ("hlt");
}

void arch_cli() {
	asm volatile ("cli");
}

void arch_sti() {
	asm volatile ("sti");
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
	if (c == '\n')
		sputc('\r');
}

char sgetc() {
	while ((inb(PORT + 5) & 1) == 0);
	return inb(PORT);
}

