#pragma once

#include <lib/multiboot.h>

void memset(u64 addr, u8 val, u64 size);
void memcpy(u64 src, u64 dest, u64 size);
