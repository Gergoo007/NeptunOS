#pragma once

#include <types.hh>

struct BitmapCommon {
	u64* buffer;
	u64 size;

	BitmapCommon(u64* ptr, u64 _size): buffer(ptr), size(_size) {  }
	void set(u64 idx, bool value);
	bool get(u64 idx);
	u64 find_and_set();
};

template <u64 staticsize>
struct BitmapStatic : BitmapCommon {
	u64 innerbuffer[align(staticsize, 64) / 64];

	BitmapStatic();
};

struct Bitmap : BitmapCommon {
	Bitmap(u64 _size);
	Bitmap(): BitmapCommon(nullptr, 0) {  }
	~Bitmap();
	void init(u64* _buffer, u64 _size);
};
