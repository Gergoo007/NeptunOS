#include <neptunos/memory/memory.h>
#include <neptunos/globals.h>
#include <neptunos/graphics/text_renderer.h>
#include <neptunos/libk/string.h>

// Memory stats in bytes
uint64_t total_mem 		= 0;
uint64_t free_mem 		= 0;
uint64_t used_mem 		= 0;
uint64_t reserved_mem 	= 0;
uint64_t num_pages 		= 0;

static void* heap_base = NULL;
static uint64_t heap_size = 0;

void map_memory(void) {
	bm.address = info->mem_stats.heap_base;
	total_mem = info->mem_stats.total;
	num_pages = total_mem / 0x1000;
	bm.size = (num_pages / 8) + 1;
	heap_base = info->mem_stats.heap_base;
	heap_size = info->mem_stats.heap_size;
	free_mem = info->mem_stats.free;
	used_mem = info->mem_stats.used;
	reserved_mem = info->mem_stats.reserved;
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

// NOTE: Only usable for physical addresses
void lock_page(void* address, uint64_t count) {
	uint64_t page = (uint64_t) address / 4096; // hányadik page
	
	if(bm_get((uint64_t)address/0x1000)) // page is already reserved/used
		return;
	
	for (; count > 0; count--)
		bm_set(page, 1);
	
	free_mem -= 4096 * count;
	used_mem += 4096 * count;
}

// NOTE: Only usable for physical addresses
void reserve_page(void* address, uint64_t count) {
	uint64_t page = (uint64_t) address / 4096; // hányadik page
	
	if(bm_get((uint64_t)address/0x1000)) // page is already reserved/used
		return;
	
	for (; count > 0; count--)
		bm_set(page, 1);

	free_mem -= 4096 * count;
	reserved_mem += 4096 * count;
}

// NOTE: Only usable for physical addresses
void unreserve_page(void* address, uint64_t count) {
	uint64_t page = (uint64_t) address / 4096; // hányadik page
	
	if(!bm_get((uint64_t)address/0x1000)) // page is already free
		return;
	
	for (; count > 0; count--)
		bm_set(page, 0);
	
	free_mem += 4096 * count;
	reserved_mem -= 4096 * count;
}

void* request_page(void) {
	// TODO: add bitmap size below
	uint64_t page = ((uint64_t)heap_base / 4096);
	while (page) { // could be while 1
		if (!bm_get(page)) {
			lock_page((void*)(page * 4096), 1);
			return (void*)(page * 4096);
		} else {
			page++;
			if ((page * 4096) > (heap_size * (uint64_t)heap_base)) {
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
	if ((((uint8_t*)bm.address)[byteIndex] & bitIndexer) > 0){
		return 1;
	}
	return 0;
}

void bm_set(uint64_t page, uint8_t val) {
	uint64_t byteIndex = page / 8;
	uint8_t bitIndex = page % 8;
	uint8_t bitIndexer = 0b00000001 << bitIndex;
	if(val)
		((uint8_t*)bm.address)[byteIndex] |= bitIndexer;
	else
		((uint8_t*)bm.address)[byteIndex] &= ~bitIndexer;
}

void* malloc(uint64_t size_in_bytes) {
	if(!size_in_bytes)
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
	return (u8*)ret + sizeof(mem_block_hdr_t);
}

void* calloc(size_t members, size_t member_size) {
	return malloc(members * member_size);
}

void free(void* addr) {
	mem_block_hdr_t* hdr = (mem_block_hdr_t*)(addr - sizeof(mem_block_hdr_t));
	free_page((u8*)addr - sizeof(mem_block_hdr_t), hdr->pages);
}
