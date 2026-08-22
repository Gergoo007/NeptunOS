#pragma once

#include <types.hh>
#include <util/async.hh>
#include <util/mem.hh>
#include <config.hh>

struct Bitmap {
	u64* buffer = nullptr;
	u64 sz;

	void set(u64 idx, bool val) {
		u64 bufidx = idx / 64;
		u64 bitidx = idx % 64;

		if constexpr (DBG) {
			if (idx >= sz)
				fatal("%s out of bounds! (%llu > %llu)", __PRETTY_FUNCTION__, idx, sz);
		}

		if (val)
			buffer[bufidx] |= (1ULL << bitidx);
		else
			buffer[bufidx] &= ~(1ULL << bitidx);
	}

	bool get(u64 idx) const {
		if constexpr (DBG) {
			if (idx >= sz)
				fatal("%s out of bounds! (%llu > %llu)", __PRETTY_FUNCTION__, idx, sz);
		}

		u64 bufidx = idx / 64;
		u64 bitidx = idx % 64;
		return (buffer[bufidx] & (1ULL << bitidx)) ? true : false;
	}

	// A start_at-nál kezdődik a keresés és körbe-wrappel, ezáltal a start-at utáni részt prioritizálja a algo
	u64 find_and_set(u64 start_at = 0) {
		return find_and_set_multiple(1);
	}

	u64 find_and_set_multiple(u64 n, u64 start_at = 0) {
		u64 first = allocate_range(start_at, sz, n);
		if (first != -1ull) {
			return first;
		} else {
			return allocate_range(0, start_at, n);
		}

		return -1;
	}

	void init(void* _buffer, u64 units, bool def = false) {
		buffer = (u64*)_buffer;
		sz = units;

		u64 defa = def ? -1ull : 0ull;

		sz = align_down(sz, 64);
		for (u64 i = 0; i < sz / 64; i++)
			buffer[i] = defa;
	}

private:
	u64 allocate_range(u64 start, u64 end, u64 n) {
		end = min(end, sz);
		assert(n > 0);

		// Jelenlegi 0-ás sorozat eleje; ha get(i) 1, akkor resetelve van
		u64 current = start;
		for (u64 i = start; i < end; i++) {
			if (get(i) == false) {
				if (i - current + 1 == n) {
					// Megvan, mostmár csak meg kell jelölni a biteket
					for (u64 j = current; j <= i; j++) {
						set(j, true);
					}
					return current;
				}
			} else {
				current = i + 1;
			}
		}

		return -1ull;
	}
};
