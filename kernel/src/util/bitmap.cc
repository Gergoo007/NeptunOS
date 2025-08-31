#include <util/bitmap.hh>

template <u64 staticsize>
BitmapStatic<staticsize>::BitmapStatic(): BitmapCommon(innerbuffer, staticsize) {
	for (u64 i = 0; i < align(size, 64) / 64; i++)
		buffer[i] = 0;
}

void BitmapCommon::set(u64 idx, bool val) {
	u64 bufidx = idx / 64;
	u64 bitidx = idx % 64;

	if (idx > size)
		printk("%s out of bounds! (%llu > %llu)\n", __PRETTY_FUNCTION__, idx, size);

	if (val)
		buffer[bufidx] |= (1ULL << bitidx);
	else
		buffer[bufidx] &= ~(1ULL << bitidx);
}

bool BitmapCommon::get(u64 idx) {
	u64 bufidx = idx / 64;
	u64 bitidx = idx % 64;
	return (buffer[bufidx] & (1ULL << bitidx)) ? true : false;
}

u64 BitmapCommon::find_and_set() {
	for (u64 i = 0; i < size; i++) {
		if (get(i) == false) {
			set(i, true);
			return i;
		}
	}
	return -1;
}

Bitmap::Bitmap(u64 _size): BitmapCommon(new u64[_size], _size) {
	for (u64 i = 0; i < align(size, 64) / 64; i++)
		buffer[i] = 0;
}

void Bitmap::init(u64* _buffer, u64 _size) {
	buffer = _buffer;
	size = _size;
	for (u64 i = 0; i < align(size, 64) / 64; i++)
		buffer[i] = 0;
}

Bitmap::~Bitmap() {
	delete buffer;
}
