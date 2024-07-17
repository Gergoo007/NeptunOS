#include <util/bootboot.h>
#include <gfx/console.h>
#include <util/mem.h>
#include <serial/serial.h>
#include <util/printf.h>

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

	printk("Hello from Arzene\be\n");
	for (u8 i = 0; i < 29; i++) {
		printk("teszt sor %d\n", i);
	}

	asm volatile ("cli");
	while (1) { asm volatile ("hlt"); }
}
