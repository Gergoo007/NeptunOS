#include <arch/amd64/tss.hh>
#include <arch/amd64/gdt.hh>
#include <mm/vmm.hh>

void arch_tss_init(gdt_entry_t* gdt) {
	tss_t* tss = (tss_t*)pmm_alloc();
	memset(tss, 0, sizeof(*tss));

	tss->io_bm_offset = offsetof(tss_t, io_bm);

	arch_gdt_add_tss(gdt, tss);
	asm volatile ("ltr %0" :: "a"((u16)0x28));
}
