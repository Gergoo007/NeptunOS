#pragma once

#include <types.hh>

#define kmalloc4g(s) pmm4g_alloc(s, __FILE_NAME__, __LINE__)
#define krealloc4g(p, s) pmm4g_realloc(p, s)
#define kmalloc_aligned4g(s, a) pmm4g_alloc_aligned(s, a, __FILE_NAME__, __LINE__)
#define kfree4g(p) pmm4g_free(p, __FILE_NAME__, __FUNCTION__)

extern u64 pmm4g_usedmem;
extern u64 pmm4g_freemem;

void pmm4g_init(u64 heap_base, u64 size);
u32 pmm4g_count_allocs();
void* pmm4g_alloc(u64 size, const char* file, u32 line);
void* pmm4g_alloc_aligned(u64 size, u32 align, const char* file, u32 line);
void* pmm4g_realloc(void* ptr, u64 newsize);
u64 pmm4g_dump();
void pmm4g_info(void* p);
void pmm4g_free(void* p, const char* file, const char* function);
void pmm4g_print_files(void* around);
