#pragma once

#include <mm/vmm.hh>
#include <util/bitmap.hh>

template <typename T>
struct Vector {
	struct Iterator {
		T* ptr;

		Iterator(T* p): ptr(p) {  }
		T& operator*() { return *ptr; }
		T* operator->() { return ptr; }
		Iterator operator++() { ptr++; return *this; }
		Iterator operator--() { ptr--; return *this; }
		Iterator operator++(int prev) { Iterator tmp = *this; ++(*this); return tmp; }
		Iterator operator--(int prev) { Iterator tmp = *this; --(*this); return tmp; }

		friend bool operator==(const Iterator& a, const Iterator& b) { return a.ptr == b.ptr; }
		friend bool operator!=(const Iterator& a, const Iterator& b) { return a.ptr != b.ptr; }
	};

	T* data;
	u64 capacity = 8;
	u64 sizeVar = 0;

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
		if (sizeVar >= capacity)
			reserve(capacity*capacity);
		data[sizeVar++] = asd;
	}

	void find() {

	}

	u64 size() {
		return sizeVar;
	}

	Iterator begin() {
		return Iterator(data);
	}

	Iterator end() {
		return Iterator(&data[sizeVar]);
	}

	~Vector() {
		vmm::free(data);
		data = (T*)0x6767676767676767;
	}
};
