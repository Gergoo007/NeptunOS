#pragma once

#include <types.hh>
#include <util/bitmap.hh>
#include <arch/amd64/paging.hh>

extern u64 pmm_freemem, pmm_usedmem, pmm_reservedmem;
constexpr u32 PMM_PAGESIZE = P4K;
constexpr MFLAGS PMM_PAGESIZE_FLAG = MFLAGS::s4K;

extern Bitmap* pmm_bm;

void pmm_init();
void* pmm_alloc(u64 size);
void* pmm_alloc4g(u64 size);
void pmm_free(void* p);
