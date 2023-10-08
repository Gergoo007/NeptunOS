#include <paging.h>
#include <serial.h>
#include <memory.h>

page_table_t* pml4;

void map_page(u64 virt, u64 phys, u8 overwrite) {
	// indexek
	u16 pdi = (virt >> 21ULL) & 511ULL;
	u16 pdpi = (virt >> 30ULL) & 511ULL;
	u16 pml4i = (virt >> 39ULL) & 511ULL;

	// releváns PDP létezik-e?
	page_table_t* pdp;

	if (pml4->entries[pml4i].flags & 1) {
		pdp = (page_table_t*)(pml4->entries[pml4i].addr & ~(0xfff));
	} else {
		pdp = (page_table_t*)bm_get_free();
		// Le kell nullázni, különben hülyeséget fogunk beolvasni
		for (u16 i = 0; i < 0x1000; i++)
			*(u8*)((u64)pdp + i) = 0x00;

		pml4->entries[pml4i].addr = (u64)pdp;
		pml4->entries[pml4i].flags |= 0b11;
	}

	// releváns PD létezik-e?
	page_table_t* pd;
	if (pdp->entries[pdpi].flags & 1) {
		pd = (page_table_t*)(pdp->entries[pdpi].addr & ~(0xfff));
	} else {
		pd = (page_table_t*)bm_get_free();
		// Le kell nullázni, különben hülyeséget fogunk beolvasni
		for (u16 i = 0; i < 0x1000; i++)
			*(u8*)((u64)pd + i) = 0x00;

		pdp->entries[pdpi].addr = (u64)pd;
		pdp->entries[pdpi].flags |= 0b11;
	}

	// Page jelen van-e?
	if (pd->entries[pdi].flags & 1) {
		if (overwrite) {
			pd->entries[pdi].addr = phys & ~(0x1fffff);
			pd->entries[pdi].flags |= 0b10000011;
		}
	} else {
		pd->entries[pdi].addr = phys & ~(0x1fffff);
		pd->entries[pdi].flags |= 0b10000011;
	}
}
