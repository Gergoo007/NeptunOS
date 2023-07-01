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

void* heap_base = NULL;
u64 heap_size = 0;

bitmap_t phys_mem_bm;

void map_memory(void) {
	phys_mem_bm.address = info->mem_stats.heap_base;
	total_mem = info->mem_stats.total;
	num_pages = total_mem / 0x1000;
	phys_mem_bm.size = (num_pages / 8) + 1;
	heap_base = info->mem_stats.heap_base;
	heap_size = info->mem_stats.heap_size;
	free_mem = info->mem_stats.free;
	used_mem = info->mem_stats.used;
	reserved_mem = info->mem_stats.reserved;

	serprintk("free: %d\n", free_mem/1024);
}

void free_page(void* address, u64 count) {
	u64 page = (u64) address / 4096; // hányadik page

	if(!bm_get(&phys_mem_bm, (u64)address/0x1000)) // page is already free
		return;

	free_mem += 4096 * count;
	used_mem -= 4096 * count;

	for (u64 i = 0; i < count; i++) {
		bm_set(&phys_mem_bm, page, 0);
	}
}

// NOTE: Only usable for physical addresses
void lock_page(void* address, u64 count) {
	u64 page = (u64) address / 4096; // hányadik page
	
	if(bm_get(&phys_mem_bm, (u64)address/0x1000)) // page is already reserved/used
		return;

	free_mem -= 4096 * count;
	used_mem += 4096 * count;

	for (; count > 0; count--)
		bm_set(&phys_mem_bm, page, 1);
}

// NOTE: Only usable for physical addresses
void reserve_page(void* address, u64 count) {
	u64 page = (u64) address / 4096; // hányadik page
	
	if(bm_get(&phys_mem_bm, (u64)address/0x1000)) // page is already reserved/used
		return;

	free_mem -= 4096 * count;
	reserved_mem += 4096 * count;

	for (; count > 0; count--)
		bm_set(&phys_mem_bm, page, 1);
}

// NOTE: Only usable for physical addresses
void unreserve_page(void* address, u64 count) {
	u64 page = (u64) address / 4096; // hányadik page
	
	if(!bm_get(&phys_mem_bm, (u64)address/0x1000)) // page is already free
		return;

	free_mem += 4096 * count;
	reserved_mem -= 4096 * count;

	for (; count > 0; count--)
		bm_set(&phys_mem_bm, page, 0);
}

void* request_page(void) {
	u64 start = (u64)heap_base / 0x1000;
	u64 hit = bm_find_clear(&phys_mem_bm, start);
	lock_page((void*)(hit*0x1000), 1);
	return (void*)(hit*0x1000);
}

void* request_pages(u32 pages) {
	void* ret = request_page();
	pages--;
	for (; pages; pages--)
		request_page();
	return ret;
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
