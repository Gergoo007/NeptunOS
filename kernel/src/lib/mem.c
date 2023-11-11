#include <lib/mem.h>

void memset(u64 addr, u8 val, u64 size) {
	for (; size; size--) {
		*(u8*)(addr++) = val;
	}
}

void memcpy(u64 src, u64 dest, u64 size) {
	for (; size; size--) {
		*(u8*)(dest++) = *(u8*)(src++);
	}
}
