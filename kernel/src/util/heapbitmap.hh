#pragma once

#include <util/bitmap.hh>
#include <util/storage.hh>

struct HeapBitmap : BitmapSkeleton {
	Vector<u64> data;

	HeapBitmap(u64 elems, bool defa = false): data(align(elems, 64) / 64) {
		init(data.data, elems, defa);
	}

	void resize(u64 newsize) {
		sz = newsize;
		data.resize(align(newsize, 64) / 64);
		buffer = data.data;
	}
};

