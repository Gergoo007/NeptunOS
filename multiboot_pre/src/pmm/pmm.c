// Basic page frame allocator

#include "pmm.h"

u64 free_mem = 0;
u64 used_mem = 0;
u64 reserved_mem = 0;

bitmap_t* bm;

void init_pmm(multiboot_tag_efi_mmap_t* mmap_tag) {
	// initialize struct
	bm->address = heap_base;
	bm->size = (total/0x1000 / 8) + 1;
	if(heap_base == NULL) {
		printk("Invalid heap_base!\n\r");
	}

	// zero the bitmap
	memset(bm->address, 0, bm->size);

	u16 num_entries = (mmap_tag->size - sizeof(multiboot_tag_efi_mmap_t)) / mmap_tag->descr_size;
	efi_mem_desc_t* desc = mmap_tag->efi_mmap;

	for (efi_mem_desc_t* desc = mmap_tag->efi_mmap;
	(u64)desc < ((u64)mmap_tag->efi_mmap + (mmap_tag->descr_size * num_entries));
	desc = (efi_mem_desc_t*) ((u8*)desc + mmap_tag->descr_size)) {
		if(desc->type > 14)
			continue;

		switch (desc->type) {
			case EfiConventionalMemory:
			case EfiBootServicesCode:
			case EfiBootServicesData:
				// _free_page((void*)desc->phys, desc->num_pages);
				free_mem += 0x1000 * desc->num_pages;
				break;

			case EfiReservedMemoryType:
			case EfiUnusableMemory:
			case EfiMemoryMappedIO:
			case EfiMemoryMappedIOPortSpace:
			case EfiPersistentMemory:
			case EfiMaxMemoryType:
			case EfiPalCode:
				// _reserve_page((void*)desc->phys, desc->num_pages);
				bm_set(desc->phys/0x1000, 1);
				reserved_mem += 0x1000 * desc->num_pages;
				break;

			default:
				// _lock_page((void*)desc->phys, desc->num_pages);
				bm_set(desc->phys/0x1000, 1);
				used_mem += 0x1000 * desc->num_pages;
				break;
		}
	}

	lock_page(bm->address, bm->size / 0x1000 + 1);
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

void* request_page() {
	u64 page = (u64)heap_base / 4096;
	while (page) { // could be while 1
		if (!bm_get(page)) {
			lock_page((void*)(page * 4096), 1);
			return (void*)(page * 4096);
		} else {
			page++;
			if ((page * 4096) > (heap_size * (u64)heap_base)) {
				printk("Out of memory!\n\r");
				while(1);
			}
		}
	}
	return NULL;
}

u8 bm_get(u64 page) {
	u64 byteIndex = page / 8;
	u8 bitIndex = page % 8;
	u8 bitIndexer = 0b00000001 << bitIndex;
	if ((((u8*)bm->address)[byteIndex] & bitIndexer) > 0){
		return 1;
	}
	return 0;
}

void bm_set(u64 page, u8 val) {
	u64 byteIndex = page / 8;
	u8 bitIndex = page % 8;
	u8 bitIndexer = 0b00000001 << bitIndex;
	if(val)
		((u8*)bm->address)[byteIndex] |= bitIndexer;
	else
		((u8*)bm->address)[byteIndex] &= ~bitIndexer;
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
	return ret + sizeof(mem_block_hdr_t);
}

void* calloc(size_t members, size_t member_size) {
	return malloc(members * member_size);
}

void free(void* addr) {
	mem_block_hdr_t* hdr = (mem_block_hdr_t*)(addr - sizeof(mem_block_hdr_t));
	free_page(addr - sizeof(mem_block_hdr_t), hdr->pages);
}
