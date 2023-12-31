#pragma once

#include <lib/int.h>

#include <graphics/console.h>

#include <memory/heap/pmm.h>

#pragma pack(1)

typedef struct page_table_entry {
	union {
		u16 flags;
		u64 addr;
	};
} page_table_entry_t;

typedef struct page_table {
	page_table_entry_t entries[512];
} page_table_t;

extern page_table_t* pml4;

void paging_init(void);
u64 paging_lookup(u64 virt);
u64 paging_lookup_2m(u64 virt);
void map_page(u64 virt, u64 phys, u32 flags);
void unmap_page(u64 virt);
