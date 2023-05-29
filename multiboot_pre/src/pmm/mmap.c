// Interpret MMAP

#include "mmap.h"

void* heap_base = NULL;
u64 heap_size = 0;
u64 total = 0;

void interpret_mmap(multiboot_tag_efi_mmap_t* mmap_tag) {
	u16 num_entries = (mmap_tag->size - sizeof(multiboot_tag_efi_mmap_t)) / mmap_tag->descr_size;
	efi_mem_desc_t* desc = mmap_tag->efi_mmap;

	for (u16 i = 0; i < num_entries; i++) {
		// Types bigger than 14 are reserved for the vendor
		if(desc->type > 14)
			goto end;

		switch (desc->type) {
			case EfiReservedMemoryType:
			case EfiUnusableMemory:
			case EfiMemoryMappedIO:
			case EfiMemoryMappedIOPortSpace:
			case EfiPersistentMemory:
			case EfiMaxMemoryType:
			case EfiPalCode:
				goto end;
			default:
				total += desc->num_pages*0x1000;
				break;
		}

		// Check if it fits in the region and is lower than 4G
		// Bitmap should only account for those 4Gs
		// Then the bitmap will be 132 KiBs
		if (desc->type == EfiConventionalMemory && desc->num_pages*0x1000 > heap_size && (desc->phys + (133*1024)) < 4294968000) {
			heap_base = (void*)desc->phys;
			heap_size = desc->num_pages*0x1000;
		}

end:	desc = (efi_mem_desc_t*) ((u8*)desc + mmap_tag->descr_size);
	}

	if(!heap_base) {
		printk("No suitable heap was found!\n\r");
		while(1);
	}

	printk("Detected memory: %d MiB\n\r", total / 1024 / 1024);
	printk("Heap size: %d KiB\n\r", heap_size / 1024);

	init_pmm(mmap_tag);
}
