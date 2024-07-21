#include <util/bootboot.h>
#include <gfx/console.h>
#include <util/mem.h>
#include <serial/serial.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/paging.h>
#include <arch/x86/idt.h>
#include <arch/x86/gdt.h>

// TODO: CPU bővítmények felderítése/feljegyzése, SSE/AVX engedélyezése

_attr_noret void kmain() {
	// Fő magon fut? Ha nem, akkor while(1)
	u32 ebx;
	asm volatile ("cpuid" : "=b"(ebx) : "a"(1));
	if (((ebx >> 24) & 0xffff) != bootboot.bspid) while (1);

	// Framebuffer előkészítése
	fb_main.base = (u64)&fb;
	fb_main.size = bootboot.fb_size;
	fb_main.width = bootboot.fb_width;
	fb_main.height = bootboot.fb_height;
	con_init();

	pmm_init();
	asm volatile ("movq %%cr3, %0" : "=r"(pml4));
	vmm_init();

	gdt_init();
	idt_init();

	*(u8*)0x8000000000000000 = 0xff;

	asm volatile ("cli");
	while (1) { asm volatile ("hlt"); }
}
