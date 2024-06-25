#include <arch/x86/tss.h>
#include <arch/x86/gdt.h>

void tss_init() {
	tss* _tss = pmm_alloc_page();

	memset(_tss, 0, sizeof(tss));

	u64 stack;
	asm volatile ("movq %%rsp, %0" : "=a"(stack));
	_tss->rsp0 = stack;
	_tss->bm_offset = sizeof(tss);

	gdt_add_tss(_tss);

	asm volatile ("mov $0x28, %ax\nltr %ax");
}
