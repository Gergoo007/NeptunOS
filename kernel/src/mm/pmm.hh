#pragma once

#include <types.hh>
#include <util/bitmap.hh>

extern u64 pmm_freemem, pmm_usedmem, pmm_reservedmem;
extern void* pmm_heap_base;
extern u64 pmm_heap_size;
constexpr u32 pmm_pagesize = 0x200000; // mibs2bytes(2)

extern bitmap_t* pmm_bm;

void pmm_init();
void* pmm_alloc(u64 size = pmm_pagesize);
void pmm_free(void* p);
