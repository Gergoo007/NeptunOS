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

extern multiboot_tag_framebuffer_t* fb_tag;

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
		pde.flags = _flags;
		pd->entries[pd_i] = pde;
	} else {
		pt = (page_map_level*)((u64)pde.address << 12);
	}

	pde = pt->entries[pt_i];
	pde.address = ((u64)physical_address >> 12);
	pde.flags = _flags;
	pt->entries[pt_i] = pde;

	__asm__ volatile ("invlpg (%0)" :: "b"(virtual_address));
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

	// Map the preloader...
	// Assume that the preloader fits in 256 pages (1M)
	void* preloader_addr = (void*)(1024*1024);
	map_region(&PSTART, &PSTART, &PEND - &PSTART + 1, MAP_FLAGS_DEFAULTS);

	// The heap including the kernel
	// We don't need the full heap, only about 256M of it
	if (heap_size <= 256*1024*1024)
		map_region(heap_base, heap_base, heap_size/0x1000 + 1, MAP_FLAGS_DEFAULTS);
	else
		map_region(heap_base, heap_base, 256*1024*1024/0x1000 + 1, MAP_FLAGS_DEFAULTS);

	// map_region(heap_base, heap_base, heap_size/0x1000 + 1, MAP_FLAGS_DEFAULTS);

	// And the multiboot tags
	map_region(mbi, mbi, mb_hdr_length + 1, MAP_FLAGS_DEFAULTS);

	// Map the framebuffer to 0xFFFFFF8040000000
	{
		void* fb = (void*)0xFFFFFF8040000000;
		u64 fb_size = fb_tag->common.framebuffer_pitch * fb_tag->common.framebuffer_height;
		map_region(fb, (void*)fb_tag->common.framebuffer_addr, fb_size/0x1000+1, MAP_FLAGS_IO_DEFAULTS);

		fb_base = fb;
	}

	// Identity map the IO ports
	map_region((void*)0x3f8, (void*)0x4e8, 0x4e8-0x3f8 / 0x1000 + 1, MAP_FLAGS_IO_DEFAULTS);

	map_page(0, 0, MAP_FLAGS_DEFAULTS);

	// Identity map current IDT
	{
		void* idtr;
		__asm__ volatile("sidt %0" : "=m"(idtr));
		idtr = (void*)((u64)idtr >> 16);
		idtr -= 0x2000;
		printk("Mapping IDT: %p\n\r", idtr);
		map_region(idtr, idtr, 2, MAP_FLAGS_DEFAULTS);
	}

	__asm__("movq %0, %%cr3" :: "a"(pml4));
}
