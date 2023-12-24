#include <memory/paging/paging.h>

page_table_t* pml4;

void paging_init(void) {
	asm volatile("movq %%cr3, %0" : "=a"(pml4));
}

u64 paging_lookup(u64 virt) {
	// indexek
	u16 pdi = (virt >> 21ULL) & 511ULL;
	u16 pdpi = (virt >> 30ULL) & 511ULL;
	u16 pml4i = (virt >> 39ULL) & 511ULL;

	page_table_t* pdp = (page_table_t*)(pml4->entries[pml4i].addr & ~0x1fffff);
	page_table_t* pd = (page_table_t*)(pdp->entries[pdpi].addr & ~0x1fffff);
	return pd->entries[pdi].addr & ~0x1fffff;
}

void map_page(u64 virt, u64 phys, u32 flags) {
	// indexek
	u16 pti = (virt >> 12ULL) & 511ULL;
	u16 pdi = (virt >> 21ULL) & 511ULL;
	u16 pdpi = (virt >> 30ULL) & 511ULL;
	u16 pml4i = (virt >> 39ULL) & 511ULL;

	page_table_t* pdp;
	page_table_t* pd;
	page_table_t* pt;

	page_table_entry_t* entry;

	entry = &pml4->entries[pml4i];
	if (entry->flags & 1) {
		pdp = (page_table_t*) (pml4->entries[pml4i].addr & ~0xfff);
	} else {
		pdp = (page_table_t*)pmm_alloc_page();
		memset(pdp, 0, 0x1000);
		pml4->entries[pml4i].addr = (u64)pdp;
		pml4->entries[pml4i].flags |= 3;
	}

	entry = &pdp->entries[pdpi];
	if (entry->flags & 1) {
		pd = (page_table_t*) (pdp->entries[pdpi].addr & ~0xfff);
	} else {
		pd = (page_table_t*)pmm_alloc_page();
		memset(pd, 0, 0x1000);
		pdp->entries[pdpi].addr = (u64)pd;
		pdp->entries[pdpi].flags |= 3;
	}

	entry = &pd->entries[pdi];
	if (entry->flags & 1) {
		pt = (page_table_t*) (pd->entries[pdi].addr & ~0xfff);
	} else {
		pt = (page_table_t*)pmm_alloc_page();
		memset(pt, 0, 0x1000);
		pd->entries[pdi].addr = (u64)pt;
		pd->entries[pdi].flags |= 3;
	}

	entry = &pt->entries[pti];
	if (entry->flags & 1) {
	} else {
		entry->addr = phys;
		entry->flags |= 3;
	}

	asm volatile ("invlpg (%0)" :: "b"(virt));
}

void unmap_page(u64 virt) {
	// indexek
	u16 pti = (virt >> 12ULL) & 511ULL;
	u16 pdi = (virt >> 21ULL) & 511ULL;
	u16 pdpi = (virt >> 30ULL) & 511ULL;
	u16 pml4i = (virt >> 39ULL) & 511ULL;

	page_table_t* pdp;
	page_table_t* pd;
	page_table_t* pt;

	page_table_entry_t* entry;

	entry = &pml4->entries[pml4i];
	if (entry->flags & 1)
		pdp = (page_table_t*) (pml4->entries[pml4i].addr & ~0xfff);
	else
		return;

	entry = &pdp->entries[pdpi];
	if (entry->flags & 1)
		pd = (page_table_t*) (pdp->entries[pdpi].addr & ~0xfff);
	else
		return;

	entry = &pd->entries[pdi];
	if (entry->flags & 1)
		pt = (page_table_t*) (pd->entries[pdi].addr & ~0xfff);
	else
		return;
	
	entry = &pt->entries[pti];
	if (entry->flags & 1)
		entry->flags &= 0xfffe;
	else
		return;

	asm volatile ("invlpg (%0)" :: "b"(virt));
}
