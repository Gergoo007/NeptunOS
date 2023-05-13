#include "paging.h"

const u16 MAP_FLAG_PRESENT = 			0b000000000001;
const u16 MAP_FLAG_RW = 				0b000000000010;
const u16 MAP_FLAG_SUPER =				0b000000000100;
const u16 MAP_FLAG_WRITE_THROUGH = 	0b000000001000;
const u16 MAP_FLAG_CACHE_DISABLE =		0b000000010000;
const u16 MAP_FLAG_ACCESSED = 			0b000000100000;
const u16 MAP_FLAG_AVAILABLE = 		0b000001000000;
const u16 MAP_FLAG_4MB_PAGES = 		0b000010000000;
const u16 MAP_FLAGS_DEFAULTS =			0 | MAP_FLAG_PRESENT | MAP_FLAG_RW;
const u16 MAP_FLAGS_IO_DEFAULTS =		0 | MAP_FLAG_PRESENT | MAP_FLAG_RW | MAP_FLAG_CACHE_DISABLE;

page_map_level* pml4;

void indexer(void* address, u16* pt_i, u16* pd_i, u16* pdp_i, u16* pml4_i) {
	u64 page_num = (u64)address / 0x1000; // The page which we want to index
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
		pde.flags = MAP_FLAG_PRESENT | MAP_FLAG_RW;
		pd->entries[pd_i] = pde;
	} else {
		pt = (page_map_level*)((u64)pde.address << 12);
	}

	pde = pt->entries[pt_i];
	pde.address = ((u64)physical_address >> 12);
	pde.flags = _flags;
	pt->entries[pt_i] = pde;
}

void map_region(void* virtual_address, void* physical_address, u64 pages, u16 _flags) {
	while(pages > 0) {
		map_page((void*)((u64)virtual_address + (pages*0x1000)-1),
			(void*)((u64)physical_address + (pages*0x1000)-1), _flags);
		pages--;
	}
}

void setup_paging() {
	pml4 = request_page();
	memset(pml4, 0, 0x1000);
	map_region(0, 0, (total/0x1000) - 1, MAP_FLAGS_DEFAULTS);

	void* teszt_page = request_page();
	void* mapped = (void*)0xFFFFFF8000000000;

	map_page(mapped, teszt_page, MAP_FLAGS_DEFAULTS);

	__asm__("movq %0, %%cr3" :: "a"(pml4));
}
