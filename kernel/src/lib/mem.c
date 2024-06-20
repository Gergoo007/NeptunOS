#include <lib/mem.h>

extern void sprintk(const char *fmt, ...);

void memset(void* addr, u8 val, u64 size) {
	for (; size; size--) {
		*(u8*)(addr++) = val;
	}
}

void memcpy(void* src, void* dest, u64 size) {
	for (; size; size--) {
		*(u8*)(dest++) = *(u8*)(src++);
	}
}
