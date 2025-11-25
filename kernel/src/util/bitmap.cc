#include <util/bitmap.hh>
#include <util/mem.hh>
#include <config.hh>

void bitmap_t::set(u64 idx, bool val) {
	u64 bufidx = idx / 64;
	u64 bitidx = idx % 64;

	if constexpr (debug) {
		if (idx > size)
			fatal("%s out of bounds! (%llu > %llu)", __PRETTY_FUNCTION__, idx, size);
	}

	if (val)
		buffer[bufidx] |= (1ULL << bitidx);
	else
		buffer[bufidx] &= ~(1ULL << bitidx);
}

bool bitmap_t::get(u64 idx) {
	u64 bufidx = idx / 64;
	u64 bitidx = idx % 64;
	return (buffer[bufidx] & (1ULL << bitidx)) ? true : false;
}

u64 bitmap_t::find_and_set() {
	for (u64 i = 0; i < size; i++) {
		if (get(i) == false) {
			sprintk("found %lld", i);
			set(i, true);
			return i;
		}
	}
	return -1;
}

void bitmap_t::init(void* _buffer, u64 units) {
	buffer = (u64*)_buffer;
	size = units;

	size = align(size, 64);
	for (u64 i = 0; i < size / 64; i++)
		buffer[i] = 0;
}
