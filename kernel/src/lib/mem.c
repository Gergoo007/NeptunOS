#include <lib/mem.h>

void memset(void* addr, u8 val, u64 size) {
	for (; size; size--) {
		*(u8*)(addr++) = val;
	}
}

void memcpy(void* src, u64 dest, u64 size) {
	for (; size; size--) {
		*(u8*)(dest++) = *(u8*)(src++);
	}
}
