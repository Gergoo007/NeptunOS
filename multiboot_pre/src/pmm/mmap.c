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
		// Check if it fits in the region and is lower than 4G
		// Bitmap should only account for those 4Gs
		// Then the bitmap will be 132 KiBs
		if (desc->type == EfiConventionalMemory && desc->num_pages*0x1000 > 133*1024 && (desc->phys + (133*1024)) < 4294968000) {
			heap_base = (void*)desc->phys;
			heap_size = desc->num_pages*0x1000;
		}

		printk("%s: %p %d KiB\n\r", EFI_MEMORY_TYPE_STRINGS[desc->type], desc->phys, desc->num_pages*0x1000 / 1024);
		
		desc = (efi_mem_desc_t*) ((uint8_t*)desc + mmap_tag->descr_size);
	}

	printk("Detected memory: %d MiB\n\r", total / 1024 / 1024);

	init_pmm(mmap_tag);
}
