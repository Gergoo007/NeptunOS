#pragma once

#include <neptunos/libk/stdint.h>

typedef struct bitmap_t {
	void* address;
	u64 size; // Size in bytes
} bitmap_t;

u8 bm_get(bitmap_t* bm, u64 index);
void bm_set(bitmap_t* bm, u64 index, u8 val);
u64 bm_find_clear(bitmap_t* bm, u64 start_index);

#include <neptunos/libk/stdall.h>
