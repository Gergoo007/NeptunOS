#pragma once

#include <types.hh>
#include <mm/pmm.hh>
#include <util/mem.hh>
#include <util/bitmap.hh>

#define kmalloc(size) vmm::alloc(size, __FILE__, __LINE__)
#define kmalloc_aligned(size) vmm::alloc_aligned(size, __FILE__, __LINE__)
#define krealloc(ptr, size) vmm::realloc(ptr, size)
#define kfree(ptr) vmm::free(ptr, __FILE__, __PRETTY_FUNCTION__)

namespace vmm {
	struct Link {
		Link* next = nullptr;
		Link* prev = nullptr;

		#ifdef DEBUG
		const char* file;
		u32 line;
		#endif

		u64 length;
		bool free;
	};

	// 16 terabyte-al a HHDM kezdete után
	constexpr u64 heap_base = 0xffff900000000000;

	void init();
	void* alloc(u64 size, const char* file, u32 line);
	void* alloc_aligned(u64 size, u32 align, const char* file, u32 line);
	void* realloc(void* ptr, u64 newsize);
	void free(void* p, const char* file, const char* function);
	u64 dump();
}
