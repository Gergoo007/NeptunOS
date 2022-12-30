#include "paging.h"
#include "../libk/string.h"
#include "../graphics/text_renderer.h"

page_map_level_4* pml4;

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

void map_address(void* virtual_address, void* physical_address) {
	// Indexes
	// pt_i points to the page, not the page table
	uint16_t pt_i, pd_i, pdp_i, pml4_i;
	indexer(virtual_address, &pt_i, &pd_i, &pdp_i, &pml4_i);

	page_table* pt;
	page_dir* pd;
	page_dir_pointers* pdp;

	// Create missing tables
	if(pml4->entries[pml4_i].present && pml4->entries[pml4_i].read_write) {
		// Entry is available, so continue
		pdp = (page_dir_pointers*)(pml4->entries[pml4_i].address << 12);
	} else {
		// Entry is not available, create it and program it
		// Zero all flags and the address
		// Create pdp
		pdp = (page_dir_pointers*) request_page();
		memset(pdp, 0, 0x1000);	

		memset(&(pml4->entries[pml4_i]), 0, 0x1000);
		
		// Register it
		// The address is 64 bits but the entry is only 52, so
		// we need to clear the 12 most significant bits (just in case)
		pml4->entries[pml4_i].address = ((uint64_t)pdp >> 12);

		// Set present and read/write
		pml4->entries[pml4_i].present = 1;
		pml4->entries[pml4_i].read_write = 1;
	}

	// Create missing tables
	if(pdp->entries[pdp_i].present && pdp->entries[pdp_i].read_write) {
		// Entry is available, so continue
		pd = (page_dir*)(pdp->entries[pdp_i].address << 12);
	} else {
		// Entry is not available, create it and program it
		// Zero all flags and the address
		// Create pd
		pd = (page_dir*) request_page();
		memset(pd, 0, 0x1000);
		memset(&(pdp->entries[pdp_i]), 0, 0x1000);

		// Register it
		// The address is 64 bits but the entry is only 52, so
		// we need to clear the 12 most significant bits (just in case)
		pdp->entries[pdp_i].address = (uint64_t)pd >> 12;

		// Set present and read/write
		pdp->entries[pdp_i].present = 1;
		pdp->entries[pdp_i].read_write = 1;
	}

	// Create missing tables
	if(pd->entries[pd_i].present && pd->entries[pd_i].read_write) {
		// Entry is available, so continue
		pt = (page_table*)(pd->entries[pd_i].address << 12);
	} else {
		// Entry is not available, create it and program it
		// Zero all flags and the address
		// Create pt
		pt = (page_table*) request_page();
		memset(&(pd->entries[pd_i]), 0, 0x1000);
		memset(pt, 0, 0x1000);

		// Register it
		// The address is 64 bits but the entry is only 52, so
		// we need to clear the 12 most significant bits (just in case)
		pd->entries[pd_i].address = (uint64_t)pt >> 12;

		// Set present and read/write
		pd->entries[pd_i].present = 1;
		pd->entries[pd_i].read_write = 1;
	}

	// Mark page as present and r/w and set the address
	pt = (page_table*)(pd->entries[pd_i].address);
	pt->entries[pt_i].present = 1;
	pt->entries[pt_i].read_write = 1;
	pt->entries[pt_i].address = (uint64_t)physical_address >> 12;

}
