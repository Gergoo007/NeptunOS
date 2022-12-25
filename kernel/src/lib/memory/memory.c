#include "memory.h"
#include "../globals.h"
#include "../graphics/text_renderer.h"
#include "../libk/string.h"

uint64_t total_mem;
uint64_t free_mem;
uint64_t used_mem;
uint64_t reserved_mem;
uint64_t num_pages;

void* biggest_conv_mem = NULL;
uint64_t biggest_conv_mem_size = 0;
uint64_t esp, ebp;
uint64_t stack_size = 8 * 1024 * 1024; // 8 MiB

void map_memory() {
	num_pages = total_mem = 0;
	asm("mov %%rsp, %0" : "=r"(esp));
	asm("mov %%rbp, %0" : "=r"(ebp));
	uint64_t stack_top = ebp - stack_size; // Because the stack grows downwards, this is actually the bottom
	uint64_t stack_pages = (stack_size / 0x1000) + 1;

	for (uint64_t i = 0; i < graphics->mMapSize / graphics->mMapDescSize; i++) {
		uint8_t match = 0;
		EFI_MEMORY_DESCRIPTOR* desc = 
			(EFI_MEMORY_DESCRIPTOR*)((uint64_t)graphics->mMap + (i * graphics->mMapDescSize));
		
		total_mem += desc->numPages * 4096;
		num_pages += desc->numPages;
		
		//printk("aaaa: %p\n", desc->physAddr);
		
		// Check for memory type
		if (desc->type == 7 && ((desc->numPages * 4096) > biggest_conv_mem_size)) {
			biggest_conv_mem = desc->physAddr;
			biggest_conv_mem_size = desc->numPages * 4096;
		}
	}
	
	//reserve_page((void*)stack_top, stack_pages);
}

void init_bitmap(bitmap* bm) {
	// initialize struct
	bm->address = biggest_conv_mem;
	bm->size = (num_pages / 8) + 1;
	mem_base = biggest_conv_mem;
	if(mem_base == NULL) {
		text_color_push(0x00ff0000); 
		printk("Invalid mem_base!\n");
		text_color_pop();
	}
	
	// zero the bitmap
	memset(bm->address, 0, bm->size);

	// fill the bitmap
	for (uint64_t i = 0; i < graphics->mMapSize / graphics->mMapDescSize; i++) {
		EFI_MEMORY_DESCRIPTOR* desc = 
			(EFI_MEMORY_DESCRIPTOR*)((uint64_t)graphics->mMap + (i * graphics->mMapDescSize));

		if(desc->type != 7)
			reserve_page(desc->physAddr, desc->numPages);
	}
	if (bm->size % 4096 != 0)
		lock_page(biggest_conv_mem, bm->size / 4096 + 1);
	else
		lock_page(biggest_conv_mem, bm->size / 4096);
}

void free_page(void* address, uint64_t count) {
	uint64_t page = (uint64_t) address / 4096; // hányadik page

	for (uint64_t i = 0; i < count; i++) {
		bm_set(page, 0);
	}
	free_mem += 4096 * count;
	used_mem -= 4096 * count;
}

void lock_page(void* address, uint64_t count) {
	uint64_t page = (uint64_t) address / 4096; // hányadik page

	for (uint64_t i = 0; i < count; i++) {
		bm_set(page, 1);
	}
	free_mem -= 4096 * count;
	used_mem += 4096 * count;
}

void reserve_page(void* address, uint64_t count) {
	uint64_t page = (uint64_t) address / 4096; // hányadik page

	for (uint64_t i = 0; i < count; i++) {
		bm_set(page, 1);
	}
	free_mem -= 4096 * count;
	reserved_mem += 4096 * count;
}

void unreserve_page(void* address, uint64_t count) {
	uint64_t page = (uint64_t) address / 4096; // hányadik page

	for (uint64_t i = 0; i < count; i++) {
		bm_set(page, 0);
	}
	free_mem += 4096 * count;
	reserved_mem -= 4096 * count;
}

void* request_page() {
	uint64_t page = (uint64_t)mem_base / 4096;
	while (page) { // could be while 1
		if (!bm_get(page)) {
			lock_page((void*)(page * 4096), 1);
			return (void*)(page * 4096);
		} else {
			page++;
			if ((page * 4096) > (biggest_conv_mem_size * (uint64_t)biggest_conv_mem)) {
				text_color_push(0x00ff0000); printk("Out of memory!\n"); text_color_pop();
			}
		}
	}
	return NULL;
}

uint8_t bm_get(uint64_t page) {
	uint64_t byteIndex = page / 8;
    uint8_t bitIndex = page % 8;
    uint8_t bitIndexer = 0b00000001 << bitIndex;
    if ((((uint8_t*)bm->address)[byteIndex] & bitIndexer) > 0){
        return 1;
    }
    return 0;
}

void bm_set(uint64_t page, uint8_t val) {
	uint64_t byteIndex = page / 8;
    uint8_t bitIndex = page % 8;
    uint8_t bitIndexer = 0b00000001 << bitIndex;
	if(val)
    	((uint8_t*)bm->address)[byteIndex] |= bitIndexer;
	else
		((uint8_t*)bm->address)[byteIndex] &= ~bitIndexer;
}

void* malloc(uint64_t size_in_bytes) {
	if(size_in_bytes == 0)
		return NULL;

	uint64_t pages = size_in_bytes / 0x1000;
	register void* ret = request_page();
	pages--;
	if(size_in_bytes == 0)
		return NULL;
	for (; pages > 0; pages--) {
		request_page();
	}
	return ret;
}
