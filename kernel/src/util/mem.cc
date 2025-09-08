#include <util/mem.hh>

void memset(void* a, const char c, u64 count) {
	u8* d = (u8*)a;

	u8 toalign = count & 7;
	while (toalign) {
		*(u8*)(d++) = c;
		toalign--;
	}
	count &= ~(7ULL);

	if (!count) return;

	u64 n = count >> 3;
	u64 c64 = (u64)c | ((u64)c << 8) | ((u64)c << 16) | ((u64)c << 24) | ((u64)c << 32) | ((u64)c << 40) | ((u64)c << 48);
	asm volatile ("rep stosq" : "=D"(d), "=c"(n) : "0"(d), "1"(n), "a"(c64) : "memory");
}

void memcpy(void* dest, void* src, u64 count) {
	u8* d = (u8*)dest;
	u8* s = (u8*)src;

	u8 toalign = count & 7;
	while (toalign) {
		*(u8*)(d++) = *(u8*)(s++);
		toalign--;
	}
	count &= ~(7ULL);

	if (!count) return;

	u64 n = count >> 3;
	asm volatile ("rep movsq"
				: "=D" (d),
				"=S" (s),
				"=c" (n)
				: "0" (d),
				"1" (s),
				"2" (n)
				: "memory");
}

bool memcmp(void* a, void* b, u64 count) {
	while (count--) {
		if (((u8*)a)[count] != ((u8*)b)[count]) return true;
	}
	return false;
}
