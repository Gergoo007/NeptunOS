#include <lib/bitmap.h>

#include <graphics/console.h>

void bm_init(bitmap_t* bm) {
	memset((void*)bm->addr, 0, bm->size);
}

void bm_set(bitmap_t* bm, u64 i, u8 val) {
	u8* byte = (u8*)(bm->addr + (i/8));
	if (val)
		*byte |= (1 << (i%8));
	else
		*byte &= ~(1 << (i%8));
}

u8 bm_get(bitmap_t* bm, u64 i) {
	u8* byte = (u8*)(bm->addr + (i/8));
	if (*byte & (1 << (i%8)))
		return 1;
	else
		return 0;
}

u64 bm_find_free(bitmap_t* bm) {
	for (u64 i = 0; i < bm->size*8; i++) {
		if (!bm_get(bm, i))
			return i;
	}

	return -1;
}

u64 bm_set_next_free(bitmap_t* bm) {
	u64 free = bm_find_free(bm);
	bm_set(bm, free, 1);
	return free;
}
