// #include "paging.h"
// #include "../libk/string.h"
// #include "../graphics/text_renderer.h"

// page_map_level* pml4;

// void indexer(void* address, uint16_t* pt_i, uint16_t* pd_i, uint16_t* pdp_i, uint16_t* pml4_i) {
// 	uint64_t page_num = (uint64_t)address / 0x1000; // The page which we want to index
// 	*pt_i = page_num % 512;
// 	page_num /= 512;
// 	*pd_i = page_num % 512;
// 	page_num /= 512;
// 	*pdp_i = page_num % 512;
// 	page_num /= 512;
// 	*pml4_i = page_num % 512;
// }

// void map_address(void* virtual_address, void* physical_address) {
// 	uint16_t pt_i, pd_i, pdp_i, pml4_i;
// 	indexer(virtual_address, &pt_i, &pd_i, &pdp_i, &pml4_i);

// 	uint64_t page_num_virt = (uint64_t)virtual_address / 0x1000;
// 	uint64_t page_num_phys = (uint64_t)physical_address / 0x1000;

// 	pml_entry tmp = pml4->entries[pml4_i];
// 	page_map_level* _pdp;
// 	if(!tmp.present) {
// 		// Create this table
// 		_pdp = (page_map_level*) request_page(); // Allocate memory for it
// 		memset(_pdp, 0, sizeof(page_map_level));
// 		tmp.address = (uint64_t)_pdp >> 12;
// 		tmp.present = 1;
// 		tmp.read_write = 1;
// 		pml4->entries[pdp_i] = tmp;
// 	} else {
// 		// The table already exists
// 		_pdp = (page_map_level*)((uint64_t)tmp.address << 12);
// 	}

// 	tmp = _pdp->entries[pdp_i];
// 	page_map_level* _pd;
// 	if(!tmp.present) {
// 		// Create this table
// 		_pd = (page_map_level*) request_page(); // Allocate memory for it
// 		memset(_pd, 0, sizeof(page_map_level));
// 		tmp.address = (uint64_t)_pd >> 12;
// 		tmp.present = 1;
// 		tmp.read_write = 1;
// 		_pdp->entries[pd_i] = tmp;
// 	} else {
// 		// The table already exists
// 		_pd = (page_map_level*)((uint64_t)tmp.address << 12);
// 	}

// 	tmp = _pd->entries[pd_i];
// 	page_map_level* _pt;
// 	if(!tmp.present) {
// 		// Create this table
// 		_pt = (page_map_level*) request_page(); // Allocate memory for it
// 		memset(_pt, 0, sizeof(page_map_level));
// 		tmp.address = (uint64_t)_pt >> 12;
// 		tmp.present = 1;
// 		tmp.read_write = 1;
// 		_pd->entries[pt_i] = tmp;
// 	} else {
// 		// The table already exists
// 		_pt = (page_map_level*)((uint64_t)tmp.address << 12);
// 	}

// 	tmp = _pt->entries[pt_i];
// 	tmp.address = (uint64_t)physical_address >> 12;
// 	tmp.present = 1;
// 	tmp.read_write = 1;
// 	_pt->entries[pt_i] = tmp;
// }
