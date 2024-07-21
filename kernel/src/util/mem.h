#pragma once

#include <util/types.h>

void memset(void* d, u8 v, u64 size);
void memcpy(void* s, void* d, u64 size);

void memset_aligned(void* d, u64 v, u64 size);
void memcpy_aligned(void* s, void* d, u64 size);
