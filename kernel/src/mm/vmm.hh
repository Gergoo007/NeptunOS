#pragma once

#include <types.hh>
#include <mm/pmm.hh>
#include <util/mem.hh>
#include <util/bitmap.hh>

#define kmalloc(size) vmm_alloc(size, __FILE__, __LINE__)
#define kmalloc_aligned(size, al) vmm_alloc_aligned(size, al, __FILE__, __LINE__)
#define krealloc(ptr, size) vmm_realloc(ptr, size)
#define kfree(ptr) vmm_free(ptr, __FILE__, __PRETTY_FUNCTION__)

// 16 terabyte-al a HHDM kezdete után
constexpr u64 vmm_heap_base = 0xffff900000000000;

extern u64 vmm_usedmem;
extern u64 vmm_freemem;

u32 vmm_count_allocs();
void vmm_init();
void* vmm_alloc(u64 size, const char* file, u32 line);
void* vmm_alloc_aligned(u64 size, u32 align, const char* file, u32 line);
void* vmm_realloc(void* ptr, u64 newsize);
void vmm_free(void* p, const char* file, const char* function);
void vmm_info(void* p);
u64 vmm_dump();


