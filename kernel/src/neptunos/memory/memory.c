#include <neptunos/memory/memory.h>
#include <neptunos/globals.h>
#include <neptunos/graphics/text_renderer.h>
#include <neptunos/libk/string.h>
#include <neptunos/memory/efi_memory.h>

// Memory stats in bytes
uint64_t total_mem 		= 0;
uint64_t free_mem 		= 0;
uint64_t used_mem 		= 0;
uint64_t reserved_mem 	= 0;
uint64_t num_pages 		= 0;

void* biggest_conv_mem = NULL;
uint64_t biggest_conv_mem_size = 0;

void map_memory() {
	for (efi_memory_descriptor_t* entry = info->mem_info->mmap; 
		(uint8_t*)entry < ((uint8_t*)info->mem_info->mmap) + info->mem_info->mmap_size; 
		entry = (efi_memory_descriptor_t*) ((uint8_t*)entry + info->mem_info->desc_size)) {

		if(entry->type == EfiConventionalMemory && entry->num_pages*0x1000 > biggest_conv_mem_size) {
			biggest_conv_mem = entry->phys_start;
			biggest_conv_mem_size = entry->num_pages*0x1000;
		}

		if(entry->type != 8) {
			total_mem += entry->num_pages*0x1000;
			num_pages += entry->num_pages;
		} else
			printk("Unusable memory detected!\n");
	}
}

void init_bitmap(bitmap_t* bm) {
	// initialize struct
	bm->address = biggest_conv_mem;
	bm->size = (num_pages / 8) + 1;
	mem_base = biggest_conv_mem;
	free_mem = total_mem;
	if(mem_base == NULL) {
		text_color_push(0x00ff0000); 
		printk("Invalid mem_base!\n");
		text_color_pop();
	}
	
	// zero the bitmap
	memset(bm->address, 0, bm->size);

	// fill the bitmap
	for (efi_memory_descriptor_t* entry = info->mem_info->mmap; 
		(uint8_t*)entry < ((uint8_t*)info->mem_info->mmap) + info->mem_info->mmap_size; 
		entry = (efi_memory_descriptor_t*) ((uint8_t*)entry + info->mem_info->desc_size)) {
		
		// If this segment is not general purpose, reserve it
		// This assumes that ExitBootServices has been called
		// (https://uefi.org/sites/default/files/resources/UEFI_Spec_2_8_final.pdf at page 232)
		if(!(entry->type == EfiBootServicesCode || entry->type == EfiBootServicesData ||
		entry->type == EfiConventionalMemory))
			reserve_page(entry->phys_start, entry->num_pages);
		else
			free_page(entry->phys_start, entry->num_pages);
	}
	
	lock_page(bm->address, bm->size / 0x1000 + 1);

	// Mark kernel pages as used
	const uint64_t kernel_num_pages = ((uint64_t)&KEND - (uint64_t)&KSTART) / 0x1000 + 1;
	lock_page(&KSTART, kernel_num_pages);
}

void free_page(void* address, uint64_t count) {
	uint64_t page = (uint64_t) address / 4096; // hányadik page

	if(!bm_get((uint64_t)address/0x1000)) // page is already free
		return;

	for (uint64_t i = 0; i < count; i++) {
		bm_set(page, 0);
	}
	free_mem += 4096 * count;
	used_mem -= 4096 * count;
}

void lock_page(void* address, uint64_t count) {
	uint64_t page = (uint64_t) address / 4096; // hányadik page
	
	if(bm_get((uint64_t)address/0x1000)) // page is already reserved/used
		return;
	
	for (uint64_t i = 0; i < count; i++) {
		bm_set(page, 1);
	}
	free_mem -= 4096 * count;
	used_mem += 4096 * count;
}

void reserve_page(void* address, uint64_t count) {
	uint64_t page = (uint64_t) address / 4096; // hányadik page
	
	if(bm_get((uint64_t)address/0x1000)) // page is already reserved/used
		return;
	
	for (uint64_t i = 0; i < count; i++) {
		bm_set(page, 1);
	}
	free_mem -= 4096 * count;
	reserved_mem += 4096 * count;
}

void unreserve_page(void* address, uint64_t count) {
	uint64_t page = (uint64_t) address / 4096; // hányadik page
	
	if(!bm_get((uint64_t)address/0x1000)) // page is already free
		return;
	
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

	// Add the size of the header
	size_in_bytes += sizeof(mem_block_hdr_t);
	
	uint64_t pages = size_in_bytes / 0x1000 + 1;
	mem_block_hdr_t block_hdr;
	block_hdr.pid = 0; // PID 0 is the kernel
	block_hdr.pages = pages;
	
	register void* ret = request_page();
	pages--;
	memcpy(ret, &block_hdr, sizeof(mem_block_hdr_t));

	for (; pages > 0; pages--)
		request_page();

	// Return the address to the allocated memory without the header
	return ret + sizeof(mem_block_hdr_t);
}

void free(void* addr) {
	mem_block_hdr_t* hdr = (mem_block_hdr_t*)(addr - sizeof(mem_block_hdr_t));
	free_page(addr - sizeof(mem_block_hdr_t), hdr->pages);
}
