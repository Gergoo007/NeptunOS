// Interpret MMAP

#include "mmap.h"

void* heap_base = NULL;
uint64_t heap_size = 0;
uint64_t total = 0;

void interpret_mmap(multiboot_tag_efi_mmap_t* mmap_tag) {
	uint16_t num_entries = (mmap_tag->size - sizeof(multiboot_tag_efi_mmap_t)) / mmap_tag->descr_size;
	efi_mem_desc_t* desc = mmap_tag->efi_mmap;
	
	for (uint16_t i = 0; i < num_entries; i++) {
		total += desc->num_pages*0x1000;

		if (desc->type == EfiConventionalMemory && desc->num_pages*0x1000 > heap_size) {
			heap_base += desc->num_pages*0x1000;
			heap_size = desc->num_pages*0x1000;
		}
		
		desc = (efi_mem_desc_t*) ((uint8_t*)desc + mmap_tag->descr_size);
	}

	printk("Detected memory: %d MiB\n\r", total / 1024 / 1024);

	init_pmm(mmap_tag);
}
