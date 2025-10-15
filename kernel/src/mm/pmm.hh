#pragma once

#include <types.hh>

namespace pmm {
	extern u64 freemem, usedmem, reservedmem;
	extern void* heap_base;
	extern u64 heap_size;
	constexpr u32 pagesize = 0x200000; // mibs2bytes(2)
	void init();
	void* alloc(u64 size = pagesize);
}
