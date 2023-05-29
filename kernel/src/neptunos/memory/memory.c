#include <neptunos/memory/memory.h>
#include <neptunos/globals.h>
#include <neptunos/graphics/text_renderer.h>
#include <neptunos/libk/string.h>

// Memory stats in bytes
u64 total_mem		= 0;
u64 free_mem		= 0;
u64 used_mem		= 0;
u64 reserved_mem	= 0;
u64 num_pages		= 0;

static void* heap_base = NULL;
static u64 heap_size = 0;

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

	serprintk("free: %d\n", free_mem/1024);
}

void free_page(void* address, u64 count) {
	u64 page = (u64) address / 4096; // hányadik page

	if(!bm_get((u64)address/0x1000)) // page is already free
		return;

	free_mem += 4096 * count;
	used_mem -= 4096 * count;

	for (u64 i = 0; i < count; i++) {
		bm_set(page, 0);
	}
}

// NOTE: Only usable for physical addresses
void lock_page(void* address, u64 count) {
	u64 page = (u64) address / 4096; // hányadik page
	
	if(bm_get((u64)address/0x1000)) // page is already reserved/used
		return;

	free_mem -= 4096 * count;
	used_mem += 4096 * count;

	for (; count > 0; count--)
		bm_set(page, 1);
}

// NOTE: Only usable for physical addresses
void reserve_page(void* address, u64 count) {
	u64 page = (u64) address / 4096; // hányadik page
	
	if(bm_get((u64)address/0x1000)) // page is already reserved/used
		return;

	free_mem -= 4096 * count;
	reserved_mem += 4096 * count;

	for (; count > 0; count--)
		bm_set(page, 1);
}

// NOTE: Only usable for physical addresses
void unreserve_page(void* address, u64 count) {
	u64 page = (u64) address / 4096; // hányadik page
	
	if(!bm_get((u64)address/0x1000)) // page is already free
		return;

	free_mem += 4096 * count;
	reserved_mem -= 4096 * count;

	for (; count > 0; count--)
		bm_set(page, 0);
}

void* request_page(void) {
	// TODO: add bitmap size below
	u64 page = ((u64)heap_base / 4096);
	while (page) { // could be while 1
		if (!bm_get(page)) {
			lock_page((void*)(page * 4096), 1);
			return (void*)(page * 4096);
		} else {
			page++;
			if ((page * 4096) > (heap_size * (u64)heap_base)) {
				text_color_push(0x00ff0000); printk("Out of memory!\n"); text_color_pop();
			}
		}
	}
	return NULL;
}

void* request_pages(u32 pages) {
	void* ret = request_page();
	pages--;
	for (; pages; pages--)
		request_page();
	return ret;
}

uint8_t bm_get(u64 page) {
	u64 byteIndex = page / 8;
	uint8_t bitIndex = page % 8;
	uint8_t bitIndexer = 0b00000001 << bitIndex;
	if ((((uint8_t*)bm.address)[byteIndex] & bitIndexer) > 0){
		return 1;
	}
	return 0;
}

void bm_set(u64 page, uint8_t val) {
	u64 byteIndex = page / 8;
	uint8_t bitIndex = page % 8;
	uint8_t bitIndexer = 0b00000001 << bitIndex;
	if(val)
		((uint8_t*)bm.address)[byteIndex] |= bitIndexer;
	else
		((uint8_t*)bm.address)[byteIndex] &= ~bitIndexer;
}

void* malloc(u64 size_in_bytes) {
	if(!size_in_bytes)
		return NULL;

	// Add the size of the header
	size_in_bytes += sizeof(mem_block_hdr_t);
	
	u64 pages = size_in_bytes / 0x1000 + 1;
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
