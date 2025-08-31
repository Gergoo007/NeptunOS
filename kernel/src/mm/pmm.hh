#pragma once

#include <types.hh>

namespace pmm {
	extern u64 free, used, reserved;
	extern void* heap_base;
	extern u64 heap_size;
	constexpr u32 pagesize = 0x1000;
	void init();
	void* alloc(u64 size = pagesize);
}
