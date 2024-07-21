#pragma once

#include <util/types.h>
#include <util/attrs.h>

typedef struct _attr_packed page_table_entry {
	union {
		u16 flags;
		u64 addr;
	};
} page_table_entry;

typedef struct page_table {
	page_table_entry entries[512];
} page_table;

extern page_table* pml4;

void paging_init(void);
u64 paging_lookup(u64 virt);
u64 paging_lookup_2m(u64 virt);
void map_page(u64 virt, u64 phys, u32 flags);
void unmap_page(u64 virt);
