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

	void init();
	void* alloc(u64 size);
	void* alloc_aligned(u64 size, u32 align);
	u64 dump();
	void free(void* p);
}
