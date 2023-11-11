#pragma once

#include <lib/int.h>
#include <lib/mem.h>

#pragma pack(1)

typedef struct bitmap {
	u64 addr;
	u64 size;
} bitmap_t;

void bm_init(bitmap_t* bm);
void bm_set(bitmap_t* bm, u64 i, u8 val);
u8 bm_get(bitmap_t* bm, u64 i);
u64 bm_find_free(bitmap_t* bm);
u64 bm_set_next_free(bitmap_t* bm);
