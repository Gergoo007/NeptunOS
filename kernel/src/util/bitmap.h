#pragma once

#include <util/types.h>

typedef struct bitmap {
	u8* base;
	u64 size; // in bytes
} bitmap;

void bm_init(bitmap* bm);
u64 bm_alloc(bitmap* bm);
void bm_set(bitmap* bm, u64 index, u8 val);
u8 bm_get(bitmap* bm, u64 index);
