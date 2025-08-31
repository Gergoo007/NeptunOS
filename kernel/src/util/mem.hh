#pragma once

#include <types.hh>

void memset(void* a, const char c, u64 count);
void memcpy(void* a, void* b);
bool memcmp(void* a, void* b, u64 count); // returns zero if equal
