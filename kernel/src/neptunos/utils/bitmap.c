#include "bitmap.h"
#include <neptunos/memory/memory.h>

u8 bm_get(bitmap_t* bm, u64 index) {
	const u64 byte = index / 8;
	const u64 bit = index % 8;

	return (*(u8*)((u64)bm->address+byte) >> bit) & 1;
}

void bm_set(bitmap_t* bm, u64 index, u8 val) {
	const u64 byte = index / 8;
	const u64 bit = index % 8;
	
	if(val)
		*(u8*)((u64)bm->address+byte) |= (val << bit);
	else
		*(u8*)((u64)bm->address+byte) &= ~(val << bit);
}

u64 bm_find_clear(bitmap_t* bm, u64 start_index) {
	for (u64 i = start_index; 1; start_index++) {
		if (!bm_get(bm, start_index)) {
			return start_index;
		}
	}
}
