#pragma once

#include <util/mm.hh>
#include <mm/pmm.hh>

#define kmalloc(size) g_vmm.alloc(size, __FILE__, __LINE__)
#define kmalloc_aligned(size, al) g_vmm.alloc_aligned(size, al, __FILE__, __LINE__)
#define krealloc(ptr, size) g_vmm.realloc(ptr, size, __FILE__, __LINE__)
#define kfree(ptr) g_vmm.free(ptr, __FILE__, __PRETTY_FUNCTION__)
#define ktryrealloc(ptr, cap) g_vmm.try_realloc(ptr, cap, __FILE__, __LINE__)

extern u8 _g_vmm[sizeof(MemoryMgr)];
extern MemoryMgr& g_vmm;

void vmm_init();

extern u64 wm_cursor, wm_free;
void* wm_alloc(u64 size, u64 align = 16);
