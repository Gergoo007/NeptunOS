#include <neptunos/memory/paging.h>
#include <neptunos/libk/string.h>
#include <neptunos/graphics/text_renderer.h>

page_map_level* pml4;

void indexer(void* address, uint16_t* pt_i, uint16_t* pd_i, uint16_t* pdp_i, uint16_t* pml4_i) {
	uint64_t page_num = (uint64_t)address / 0x1000; // The page which we want to index
	*pt_i = page_num % 512;
	page_num /= 512;
	*pd_i = page_num % 512;
	page_num /= 512;
	*pdp_i = page_num % 512;
	page_num /= 512;
	*pml4_i = page_num % 512;
}

void map_page(void* virtual_address, void* physical_address, u16 _flags) {
	// Indexes
	// pt_i points to the page, not the page table
	u16 pt_i, pd_i, pdp_i, pml4_i;
	indexer(virtual_address, &pt_i, &pd_i, &pdp_i, &pml4_i);

	page_map_entry pde;

	pde = pml4->entries[pml4_i];
	page_map_level* pdp;
	if (!pde.present) {
		pdp = (page_map_level*)request_page();
		memset(pdp, 0, 0x1000);
		pde.address = (u64)pdp >> 12;
		// pde.flags = _flags;
		pde.flags = MAP_FLAG_PRESENT | MAP_FLAG_RW;
		pml4->entries[pml4_i] = pde;
	} else {
		pdp = (page_map_level*)((u64)pde.address << 12);
	}

	pde = pdp->entries[pdp_i];
	page_map_level* pd;
	if (!pde.present) {
		pd = (page_map_level*)request_page();
		memset(pd, 0, 0x1000);
		pde.address = (u64)pd >> 12;
		// pde.flags = _flags;
		pde.flags = MAP_FLAG_PRESENT | MAP_FLAG_RW;
		pdp->entries[pdp_i] = pde;
	} else {
		pd = (page_map_level*)((u64)pde.address << 12);
	}

	pde = pd->entries[pd_i];
	page_map_level* pt;
	if (!pde.present) {
		pt = (page_map_level*)request_page();
		memset(pt, 0, 0x1000);
		pde.address = (u64)pt >> 12;
		// pde.flags = _flags;
		pde.flags = _flags;
		pd->entries[pd_i] = pde;
	} else {
		pt = (page_map_level*)((u64)pde.address << 12);
	}

	pde = pt->entries[pt_i];
	pde.address = ((u64)physical_address >> 12);
	pde.flags = _flags;
	pt->entries[pt_i] = pde;

	asm volatile ("invlpg (%0)" :: "b"(virtual_address));
}

void map_region(void* virtual_address, void* physical_address, uint64_t pages, uint16_t _flags) {
	while(pages > 0) {
		map_page((void*)((uint64_t)virtual_address + (pages*0x1000)-1),
			(void*)((uint64_t)physical_address + (pages*0x1000)-1), _flags);
		pages--;
	}
}
