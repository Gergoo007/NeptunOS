#include <arch/amd64/tss.hh>
#include <arch/amd64/gdt.hh>
#include <mm/vmm.hh>

tss_t* tss;

void arch_tss_init() {
	tss = (tss_t*)pmm_alloc();
	memset(tss, 0, sizeof(*tss));

	tss->io_bm_offset = offsetof(tss_t, io_bm);

	arch_gdt_add_tss(tss);
	asm volatile ("ltr %0" :: "a"((u16)0x28));
}
