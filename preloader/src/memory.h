#pragma once

#include <int.h>

// Egy bit egy page-et képvisel, ha 0 akkor szabad, ha nem akkor nem.
// 64 page-nél többre úgyse lesz szükség (1 GiB)
extern u64 bitmap;
extern u64 heap_base;

void bm_initialize(u64 addr);
u8 bm_is_used(u8 i);
void bm_set_used(u8 i, u8 used);
u64 bm_get_addr(u8 i);
u64 bm_get_free(void);
