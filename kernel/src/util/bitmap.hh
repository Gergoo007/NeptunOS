#pragma once

#include <types.hh>
#include <util/async.hh>
#include <util/mem.hh>
#include <config.hh>

struct bitmap_t {
	u64* buffer;
	u64 size;
	mutex m;

	void set(u64 idx, bool val) {
		lockguard turi(m);
		u64 bufidx = idx / 64;
		u64 bitidx = idx % 64;

		if constexpr (DBG) {
			if (idx > size)
				fatal("%s out of bounds! (%llu > %llu)", __PRETTY_FUNCTION__, idx, size);
		}

		if (val)
			buffer[bufidx] |= (1ULL << bitidx);
		else
			buffer[bufidx] &= ~(1ULL << bitidx);
	}

	bool get(u64 idx) {
		lockguard turi(m);
		u64 bufidx = idx / 64;
		u64 bitidx = idx % 64;
		return (buffer[bufidx] & (1ULL << bitidx)) ? true : false;
	}

	u64 find_and_set() {
		lockguard turi(m);
		// for (u64 i = 0; i < size; i++) {
		// 	if (get(i) == false) {
		// 		set(i, true);
		// 		return i;
		// 	}
		// }
		// return -1;
		for (u64 i = 0; i < size; i++) {
			u64 bufidx = i / 64;
			u64 bitidx = i % 64;
			u64 mask = (1ULL << bitidx);

			if ((buffer[bufidx] & mask) == 0) {
				buffer[bufidx] |= mask;
				return i;
			}
		}
		return -1;
	}

	void init(void* _buffer, u64 units) {
		buffer = (u64*)_buffer;
		size = units;

		size = align(size, 64);
		for (u64 i = 0; i < size / 64; i++)
			buffer[i] = 0;
	}
};
