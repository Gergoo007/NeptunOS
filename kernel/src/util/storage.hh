#pragma once

#include <mm/vmm.hh>
#include <util/bitmap.hh>

template <typename T>
struct Vector {
	T* data;
	u64 capacity = 8;
	u64 size = 0;

	Vector() {
		data = (T*)vmm::alloc(capacity * sizeof(T));
	}

	Vector(u64 cap): capacity(cap) {
		data = (T*)vmm::alloc(capacity * sizeof(T));
	}

	void reserve(u64 cap) {
		// vmm::realloc();
	}

	T& operator[](u64 idx) {
		#ifdef DEBUG
		if (idx > size)
			error("Vector (%p) out of bounds!\n", this);
		#endif
		return data[idx];
	}

	void push_back(T asd) {
		if (size >= capacity) {
			reserve(capacity*capacity);
		}
		data[size++] = asd;
	}

	~Vector() {
		vmm::free(data);
		data = (T*)0x6767676767676767;
	}
};
