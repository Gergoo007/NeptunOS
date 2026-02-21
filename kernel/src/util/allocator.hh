#pragma once

#include <types.hh>
#include <util/mm.hh>

#ifdef KERNEL
extern memorymgr& g_vmm;

#define _galloc(sz) g_vmm.alloc(sz, __FILE__, __LINE__)
#define _grealloc(ptr, sz) g_vmm.realloc(ptr, sz, __FILE__, __LINE__)
#define _gfree(sz) g_vmm.free(sz, __FILE__, __PRETTY_FUNCTION__)
#define _gtryrealloc(ptr, sz) g_vmm.try_realloc(ptr, sz, __FILE__, __LINE__)
#else
#error "Not impl."
#endif
