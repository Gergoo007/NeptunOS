#pragma once

#include <mm/pmm.hh>

#define kmalloc4g(size) pmm_alloc4g(size)
#define kmalloc_aligned4g(size, al) pmm_alloc4g(size)
#define krealloc4g(ptr, size) ptr
#define kfree4g(ptr) pmm_free(ptr)
#define ktryrealloc4g(ptr, cap) fatal("ktryrealloc4g unimpl")
