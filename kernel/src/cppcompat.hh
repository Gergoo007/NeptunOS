#pragma once

#include <types.hh>

extern void (*__init_array[])();
extern void (*__init_array_end[])();

void cpp_construct_objects();

inline void* operator new(size_t size, void* ptr) noexcept {
	return ptr;
}
