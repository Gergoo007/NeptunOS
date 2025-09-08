#pragma once

#include <types.hh>
#include <mm/pmm.hh>
#include <util/mem.hh>
#include <util/bitmap.hh>

namespace vmm {
	struct Link {
		Link* next = nullptr;
		Link* prev = nullptr;

		u64 length;
		bool free;
	};

	// 16 terabyte-al a HHDM után
	constexpr u64 heap_base = 0xffff900000000000;

	void init();
	void* alloc(u64 size);
	void* alloc_aligned(u64 size, u32 align);
	void* realloc(void* ptr, u64 newsize);
	u64 dump();
	void free(void* p);
}
