#include "paging.h"
#include "main.h"

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

void map_address(void* virtual_address, void* physical_address) {
	// Indexes
	// pt_i points to the page, not the page table
	uint16_t pt_i, pd_i, pdp_i, pml4_i;
	indexer(virtual_address, &pt_i, &pd_i, &pdp_i, &pml4_i);

	page_map_entry pde;

	pde = pml4->entries[pml4_i];
    page_map_level* pdp;
    if (!pde.present){
        pdp = (page_map_level*)malloc(0x1000);
        memset(pdp, 0, 0x1000);
        pde.address = (uint64_t)pdp >> 12;
        pde.present = 1;
        pde.read_write = 1;
        pml4->entries[pml4_i] = pde;
    }
    else
    {
        pdp = (page_map_level*)((uint64_t)pde.address << 12);
    }
    
    pde = pdp->entries[pdp_i];
    page_map_level* pd;
    if (!pde.present){
        pd = (page_map_level*)malloc(0x1000);
        memset(pd, 0, 0x1000);
        pde.address = (uint64_t)pd >> 12;
        pde.present = 1;
        pde.read_write = 1;
        pdp->entries[pdp_i] = pde;
    }
    else
    {
        pd = (page_map_level*)((uint64_t)pde.address << 12);
    }

    pde = pd->entries[pd_i];
    page_map_level* pt;
    if (!pde.present){
        pt = (page_map_level*)malloc(0x1000);
        memset(pt, 0, 0x1000);
        pde.address = (uint64_t)pt >> 12;
        pde.present = 1;
        pde.read_write = 1;
        pd->entries[pd_i] = pde;
    }
    else
    {
        pt = (page_map_level*)((uint64_t)pde.address << 12);
    }
	
	pde = pt->entries[pt_i];
    pde.address = ((uint64_t)physical_address >> 12);
    pde.present = 1;
    pde.read_write = 1;
    pt->entries[pt_i] = pde;
}
