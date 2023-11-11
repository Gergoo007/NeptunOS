#pragma once

#include <graphics/console.h>
#include <lib/multiboot.h>
#include <lib/bitmap.h>
#include <lib/kinfo.h>

typedef struct new_mmap_entry {
	u64 addr;
	u64 length;
	u64 type;
} new_mmap_entry_t;

void pmm_init(mb_tag_memmap_t* mmap);
u64 pmm_translate(u64 pseudo_addr);
u64 pmm_reverse_translate(u64 physical_addr);
u64 pmm_alloc_page(void);
