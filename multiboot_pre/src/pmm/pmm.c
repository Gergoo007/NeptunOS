// Basic page frame allocator

#include "pmm.h"

uint64_t free_mem;
uint64_t used_mem;
uint64_t reserved_mem;

bitmap_t* bm;

void init_pmm(multiboot_tag_efi_mmap_t* mmap_tag) {
	// initialize struct
	bm->address = heap_base;
	bm->size = (total/0x1000 / 8) + 1;
	free_mem = total;
	printk("Number of pages: %ud \n\r", total/0x1000);
	if(heap_base == NULL) {
		printk("Invalid heap_base!\n\r");
	}
	
	printk("Zeroing bitmap...\n\r");
	printk("Size: %ud bytes (var)\n\r", bm->size);
	printk("Will zero %p\n\r", bm->address);
	printk("Until %p\n\r", bm->address+bm->size);
	// zero the bitmap
	memset(bm->address, 0, bm->size);

	// for (uint64_t i = 0; i < bm->size; i++) {
	// 	*((uint8_t*)bm->address+i) = 0;
	// 	printk("%p\n", (uint8_t*)bm->address+i);
	// }

	*((uint8_t*)bm->address) = 0;

	printk("Asd");
	
	uint16_t num_entries = (mmap_tag->size - sizeof(multiboot_tag_efi_mmap_t)) / mmap_tag->descr_size;
	efi_mem_desc_t* desc = mmap_tag->efi_mmap;
	
	for (efi_mem_desc_t* desc = mmap_tag->efi_mmap;
	(uint64_t)desc < ((uint64_t)mmap_tag->efi_mmap + (mmap_tag->descr_size * num_entries));
	desc = (efi_mem_desc_t*) ((uint8_t*)desc + mmap_tag->descr_size)) {
		switch (desc->type) {
			case EfiConventionalMemory:
			case EfiBootServicesCode:
			case EfiBootServicesData:
				free_page((void*)desc->phys, desc->num_pages);
				break;

			case EfiACPIMemoryNVS:
			case EfiACPIReclaimMemory:
			case EfiMemoryMappedIO:
			case EfiMemoryMappedIOPortSpace:
			case EfiPersistentMemory:
			case EfiReservedMemoryType:
			case EfiUnusableMemory:
				reserve_page((void*)desc->phys, desc->num_pages);
				break;
			
			default:
				lock_page((void*)desc->phys, desc->num_pages);
				break;
		}
	}
	
	printk("Locking it...\n\r");
	lock_page(bm->address, bm->size / 0x1000 + 1);
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

void* request_page() {
	uint64_t page = (uint64_t)heap_base / 4096;
	while (page) { // could be while 1
		if (!bm_get(page)) {
			lock_page((void*)(page * 4096), 1);
			return (void*)(page * 4096);
		} else {
			page++;
			if ((page * 4096) > (heap_size * (uint64_t)heap_base)) {
				printk("Out of memory!\n\r");
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
	return ret + sizeof(mem_block_hdr_t);
}

void* calloc(size_t members, size_t member_size) {
	return malloc(members * member_size);
}

void free(void* addr) {
	mem_block_hdr_t* hdr = (mem_block_hdr_t*)(addr - sizeof(mem_block_hdr_t));
	free_page(addr - sizeof(mem_block_hdr_t), hdr->pages);
}
