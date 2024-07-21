#include <mm/paging.h>
#include <mm/pmm.h>
#include <util/mem.h>

page_table* pml4;

void paging_init(void) {
	asm volatile("movq %%cr3, %0" : "=a"(pml4));
}

u64 paging_lookup_2m(u64 virt) {
	// indexek
	u16 pdi = (virt >> 21ULL) & 511ULL;
	u16 pdpi = (virt >> 30ULL) & 511ULL;
	u16 pml4i = (virt >> 39ULL) & 511ULL;

	page_table* pdp = (page_table*)(pml4->entries[pml4i].addr & ~0x1fffff);
	page_table* pd = (page_table*)(pdp->entries[pdpi].addr & ~0x1fffff);
	return pd->entries[pdi].addr & ~0x1fffff;
}

u64 paging_lookup(u64 virt) {
	// indexek
	u16 pti = (virt >> 12ULL) & 511ULL;
	u16 pdi = (virt >> 21ULL) & 511ULL;
	u16 pdpi = (virt >> 30ULL) & 511ULL;
	u16 pml4i = (virt >> 39ULL) & 511ULL;

	page_table* pdp = (page_table*) (pml4->entries[pml4i].addr & ~0x0fff);
	page_table* pd = (page_table*) (pdp->entries[pdpi].addr & ~0x0fff);
	page_table* pt = (page_table*) (pd->entries[pdi].addr & ~0x0fff);

	return pt->entries[pti].addr & ~(0x0fff);
}

void map_page(u64 virt, u64 phys, u32 flags) {
	if (!flags) flags = 0b111;
	
	// indexek
	u16 pti = (virt >> 12ULL) & 511ULL;
	u16 pdi = (virt >> 21ULL) & 511ULL;
	u16 pdpi = (virt >> 30ULL) & 511ULL;
	u16 pml4i = (virt >> 39ULL) & 511ULL;

	page_table* pdp;
	page_table* pd;
	page_table* pt;

	page_table_entry* entry;

	entry = &pml4->entries[pml4i];
	if (entry->flags & 1) {
		pdp = (page_table*) (pml4->entries[pml4i].addr & ~0xfff);
	} else {
		pdp = (page_table*)pmm_alloc();
		memset(pdp, 0, 0x1000);
		pml4->entries[pml4i].addr = (u64)pdp;
		pml4->entries[pml4i].flags |= flags;
	}

	entry = &pdp->entries[pdpi];
	if (entry->flags & 1) {
		pd = (page_table*) (pdp->entries[pdpi].addr & ~0x0fff);
	} else {
		pd = (page_table*)pmm_alloc();
		memset(pd, 0, 0x1000);
		pdp->entries[pdpi].addr = (u64)pd;
		pdp->entries[pdpi].flags |= flags;
	}

	entry = &pd->entries[pdi];
	if (entry->flags & 1) {
		pt = (page_table*) (pd->entries[pdi].addr & ~0x0fff);
	} else {
		pt = (page_table*)pmm_alloc();
		memset(pt, 0, 0x1000);
		pd->entries[pdi].addr = (u64)pt;
		pd->entries[pdi].flags |= flags;
	}

	entry = &pt->entries[pti];
	if (entry->flags & 1) {
	} else {
		entry->addr = phys;
		entry->flags |= flags;
	}

	asm volatile ("invlpg (%0)" :: "b"(virt));
}

void unmap_page(u64 virt) {
	// indexek
	u16 pti = (virt >> 12ULL) & 511ULL;
	u16 pdi = (virt >> 21ULL) & 511ULL;
	u16 pdpi = (virt >> 30ULL) & 511ULL;
	u16 pml4i = (virt >> 39ULL) & 511ULL;

	page_table* pdp = (page_table*) (pml4->entries[pml4i].addr & ~0x0fff);
	page_table* pd = (page_table*) (pdp->entries[pdpi].addr & ~0x0fff);
	page_table* pt = (page_table*) (pd->entries[pdi].addr & ~0x0fff);

	pt->entries[pti].flags &= ~1ULL;

	asm volatile ("invlpg (%0)" :: "b"(virt));
}
