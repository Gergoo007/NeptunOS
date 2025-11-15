#pragma once

#include <types.hh>

typedef struct bitmap {
	u64* buffer;
	u64 size;

	void init(void* buffer, u64 size);
	void set(u64 idx, bool value);
	bool get(u64 idx);
	u64 find_and_set();
} bitmap_t;
