#pragma once

#include <lib/multiboot.h>

void memset(void* addr, u8 val, u64 size);
void memcpy(void* src, void* dest, u64 size);
