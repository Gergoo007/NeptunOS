#include <arch/amd64/tss.hh>
#include <arch/amd64/gdt.hh>
#include <mm/vmm.hh>

void arch_tss_init(gdt_entry_t* gdt) {
	tss_t* tss = (tss_t*)wm_alloc(128);
	memset(tss, 0, sizeof(*tss));

	// Új stack az interrupt routine-oknak, 2M
	u64 stack = (u64)pmm_alloc(INTR_STACK_SIZE) + INTR_STACK_SIZE - 128 - 16;
	tss->ist1 = stack;

	tss->io_bm_offset = offsetof(tss_t, io_bm);

	arch_gdt_add_tss(gdt, tss);
	asm volatile ("ltr %0" :: "a"((u16)0x28));
}
