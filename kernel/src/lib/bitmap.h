#pragma once

#include <lib/types.h>
#include <lib/mem.h>

#pragma pack(1)

typedef struct bitmap {
	u64 addr;
	u64 size;
} bitmap;

void bm_init(bitmap* bm);
void bm_set(bitmap* bm, u64 i, u8 val);
u8 bm_get(bitmap* bm, u64 i);
u64 bm_find_free(bitmap* bm);
u64 bm_set_next_free(bitmap* bm);
