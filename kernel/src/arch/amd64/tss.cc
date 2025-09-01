#include <arch/amd64/tss.hh>
#include <arch/amd64/gdt.hh>
#include <mm/vmm.hh>

namespace arch::tss {
	tss* tss_actual;

	void init() {
		tss_actual = (tss*)pmm::alloc();
		memset(tss_actual, 0, sizeof(tss));

		tss_actual->io_bm_offset = offsetof(tss, io_bm);

		arch::gdt::add_tss(tss_actual);
		asm volatile ("ltr %0" :: "a"((u16)0x28));
	}
}
