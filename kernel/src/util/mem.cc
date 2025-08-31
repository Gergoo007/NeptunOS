#include <util/mem.hh>

void memset(void* a, const char c, u64 count) {
	// u8* d = (u8*)a;
	// if (count < 16) {
	// 	while (count) {
	// 		d[count] = c;
	// 		count--;
	// 	}
	// 	return;
	// }

	// // Cím igazítása 64 bites határra
	// u64 i = 0;
	// while ((u64)d & 7) { *(d++) = c; i++; }

	// d = (u8*)a;
	// u64 v = c;
	// v |= (v << 8) | (v << 16) | (v << 24) | (v << 32) | (v << 48);
	// for (; i < count - 8; i += 8) {
	// 	*(u64*)&(d[i]) = v;
	// }

	// // Maradék byte-ok (a count nem biztos hogy a 8 többszöröse)
	// for (; i < count; i++) {
	// 	d[i] = c;
	// }

	for (u64 i = 0; i < count; i++) {
		((u8*)a)[i] = c;
	}
}

void memcpy(void* a, void* b) {

}

bool memcmp(void* a, void* b, u64 count) {
	while (count--) {
		if (((u8*)a)[count] != ((u8*)b)[count]) return 1;
	}
	return 0;
}
