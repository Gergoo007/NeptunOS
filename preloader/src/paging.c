#include <paging.h>
#include <serial.h>
#include <memory.h>
#include <strings.h>

page_table_t* pml4;

void map_page(u64 virt, u64 phys) {
	u8 flags = 0b111;
	// indexek
	u16 pdi = (virt >> 21ULL) & 511ULL;
	u16 pdpi = (virt >> 30ULL) & 511ULL;
	u16 pml4i = (virt >> 39ULL) & 511ULL;

	page_table_t* pdp;
	page_table_t* pd;

	if (pml4->entries[pml4i].flags & 0b1) {
		// A PDP már létezik
		pdp = (page_table_t*) (pml4->entries[pml4i].addr & ~(0xfffULL));
	} else {
		pdp = (page_table_t*) bm_get_free();
		memset((u8*)pdp, 0, 0x1000);

		pml4->entries[pml4i].addr |= (u64)pdp & ~(0xfffULL);
		pml4->entries[pml4i].flags |= flags;
	}

	if (pdp->entries[pdpi].flags & 0b1) {
		// A PD már létezik
		// pd = (page_table_t*) (pdp->entries[pdpi].addr & ~(0xfffULL));

		pd = (page_table_t*) bm_get_free();
		memset((u8*)pd, 0, 0x1000);

		pdp->entries[pdpi].addr |= (u64)pd & ~(0xfffULL);
		pdp->entries[pdpi].flags |= flags;
	} else {
		pd = (page_table_t*) bm_get_free();
		memset((u8*)pd, 0, 0x1000);

		pdp->entries[pdpi].addr |= (u64)pd & ~(0xfffULL);
		pdp->entries[pdpi].flags |= flags;
	}
	
	pd->entries[pdi].addr |= (phys & ~(PAGESIZE - 1));
	pd->entries[pdi].flags |= 0b10000000 | flags;

	// printf("%p\n\r", &pml4->entries[pml4i]);
	// printf("%p\n\r", &pdp->entries[pdpi]);
	// printf("%p\n\r", &pd->entries[pdi]);
}
