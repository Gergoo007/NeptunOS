#include <memory.h>
#include <serial.h>

// Egy bit egy page-et képvisel, ha 0 akkor szabad, ha nem akkor nem.
// 64 page-nél többre úgyse lesz szükség (1 GiB)
u64 bitmap = 0;
u64 heap_base = 0;

void bm_initialize(u64 addr) {
	// A címet át kell alakítani, hogy page határhoz legyen
	// igazítva
	if (addr % PAGESIZE) {
		heap_base = (addr+PAGESIZE) & ~(PAGESIZE-1);
	} else {
		heap_base = addr;
	}
}

u8 bm_is_used(u8 i) {
	if (bitmap & (1ULL << i)) {
		return 1;
	} else {
		return 0;
	}
}

void bm_set_used(u8 i, u8 used) {
	if (used)
		bitmap |= (1ULL << i);
	else
		bitmap &= ~(1ULL << i);
}

u64 bm_get_addr(u8 i) {
	return heap_base + (i * PAGESIZE);
}

u64 bm_get_free(void) {
	u8 i = 0;
	for (; i < 64; i++) {
		if (!bm_is_used(i)) {
			bm_set_used(i, 1);
			return bm_get_addr(i);
		}
	}

	printf("No free space left!\n\r");
	return 0;
}
