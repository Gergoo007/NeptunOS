#include <graphics/panic.h>

void panic(const char* fmt, ...) {
	con_bg = 0xff000000;
	con_fg = 0xffffffff;

	con_cx = 0;
	con_cy = 0;

	printk("Panic occured!\n");

	va_list list;
	va_start(list, fmt);
	char buf[128];
	vsprintf(buf, fmt, list);
	printk(buf);

	printk("Stacktrace:\n");

	u64 addr = 0;
	u64 nearest = 0;
	stackframe* rbp;
	asm volatile ("movq %%rbp, %0" : "=r"(rbp));
	
	while (addr != (u64)kmain) {
		addr = rbp->rip;
		nearest = sym_nearest_sym(addr);

		sprintf(buf, "%s +0x%x\n", sym_at_addr(nearest), addr - nearest);
		printk(buf);

		addr = nearest;
		rbp = rbp->rbp;
	}

	va_end(list);

	asm volatile ("cli");
	asm volatile ("hlt");
}
