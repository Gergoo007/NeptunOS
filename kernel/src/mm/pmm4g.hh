#pragma once

#include <util/mm.hh>

#define kmalloc4g(size) g_pmm4g.alloc(size, __FILE__, __LINE__)
#define kmalloc_aligned4g(size, al) g_pmm4g.alloc_aligned(size, al, __FILE__, __LINE__)
#define krealloc4g(ptr, size) g_pmm4g.realloc(ptr, size, __FILE__, __LINE__)
#define kfree4g(ptr) g_pmm4g.free(ptr, __FILE__, __PRETTY_FUNCTION__)
#define ktryrealloc4g(ptr, cap) g_pmm4g.try_realloc(ptr, cap, __FILE__, __LINE__)

extern u8 _g_pmm4g[sizeof(memorymgr)];
extern memorymgr& g_pmm4g;

void pmm4g_init(u64 heap_base, u64 size);
