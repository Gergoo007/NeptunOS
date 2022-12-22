#include "memory.h"
#include "../globals.h"
#include "../graphics/text_renderer.h"
#include "../libk/string.h"

uint64_t total_mem;
uint64_t free_mem;
uint64_t used_mem;
uint64_t reserved_mem;
uint64_t num_pages;

void map_memory() {
	num_pages = total_mem = 0;
	
	void* biggest_conv_mem = NULL;
	uint64_t biggest_conv_mem_size = 0;
	for (uint64_t i = 0; i < graphics->mMapSize / graphics->mMapDescSize; i++) {
		EFI_MEMORY_DESCRIPTOR* desc = 
			(EFI_MEMORY_DESCRIPTOR*)((uint64_t)graphics->mMap + (i * graphics->mMapDescSize));
		total_mem += desc->numPages * 4096;
		num_pages += desc->numPages;

		// Check for memory type
		if (desc->type == 7 && ((desc->numPages * 4096) > biggest_conv_mem_size)) {
			biggest_conv_mem = desc->physAddr;
			biggest_conv_mem_size = desc->numPages * 4096;
		}
	}
}

void init_bitmap(uint8_t* _bitmap, uint64_t _bitmap_size) {
	for (uint64_t i = 0; i < _bitmap_size; i++)
		_bitmap[i] = 0x00;

	bitmap = _bitmap;
	bitmap_size = _bitmap_size;
}

void free_page(void* address, uint64_t count) {
	uint64_t page_i = (uint64_t) address / 4096;
	bitmap[page_i / 8] &= 1 >> (page_i % 8);
}

void lock_page(void* address, uint64_t count) {
	uint64_t page_i = (uint64_t) address / 4096;
	bitmap[page_i / 8] &= 1 >> (page_i % 8);
}

void reserve_page(void* address, uint64_t count) {
	uint64_t page_i = (uint64_t) address / 4096;
	bitmap[page_i / 8] &= 1 >> (page_i % 8);
}
