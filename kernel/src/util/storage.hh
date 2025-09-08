#pragma once

#include <mm/vmm.hh>
#include <util/bitmap.hh>
#include <util/string.hh>

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

	T* data = nullptr;
	u64 capacity = 8;
	u64 size = 0;

	Vector() {
		data = (T*)vmm::alloc(capacity * sizeof(T));
	}

	Vector(u64 cap): capacity(cap) {
		if (size)
			data = (T*)vmm::alloc(capacity * sizeof(T));
	}

	Vector(const Vector& other) {
		size = other.size;
		capacity = other.capacity;
		data = (T*)vmm::alloc(capacity * sizeof(T));
		memcpy(data, other.data, other.size);
	}

	void reserve(u64 cap) {
		data = (T*)vmm::realloc(data, cap * sizeof(T));
	}

	T& operator[](u64 idx) {
		#ifdef DEBUG
		if (idx > size)
			error("Vector (%p) out of bounds!\n", this);
		#endif
		return data[idx];
	}

	void push_back(T asd) {
		if (size >= capacity)
			reserve(capacity*capacity);
		data[size++] = asd;
	}

	void find() {

	}

	Iterator begin() {
		return Iterator(data);
	}

	Iterator end() {
		return Iterator(&data[size]);
	}

	~Vector() {
		vmm::free(data);
		// M A S O N
		data = (T*)0x6767676767676767;
	}
};

struct String : Vector<char> {
	String(): Vector<char>(8) {  }
	String(u32 _size): Vector<char>(_size) {  }
	String(const char* s): Vector<char>(strlen(s) + 1) {
		memcpy((void*)data, (void*)s, capacity);
		size = capacity;
		capacity = align(capacity, 16);
		data[size] = 0;
	}
	String(const char* s, u64 strsize): Vector<char>(strsize + 1) {
		memcpy((void*)data, (void*)s, capacity);
		size = capacity-1;
		capacity = align(capacity, 16);
		data[size] = 0;
	}
	String(const String& str): Vector<char>(str) {  }
	String& operator=(const String& to) {
		if (data)
			vmm::free(data);
		size = to.capacity;
		size = to.size;
		data = (char*)vmm::alloc(capacity);
		memcpy(data, to.data, size);
		data[size] = 0;
		return *this;
	}
	String& operator=(const char* s) {
		u32 chars = strlen(s);
		size = chars + 1;
		capacity = align(chars, 16);
		if (data)
			vmm::free(data);
		data = (char*)vmm::alloc(capacity);
		memcpy(data, (void*)s, chars);
		data[size] = 0;
		return *this;
	}

	inline char* c_str() { return data; }
};

template <typename T>
struct Stack {
	T* data = nullptr;
	u64 size = 0;
	u64 capacity = 16;

	Stack(u64 cap = 16): capacity(cap) {
		data = (T*)vmm::alloc(capacity * sizeof(T));
	}

	void reserve(u64 cap) {
		capacity = cap;
		data = (T*)vmm::realloc(data, capacity * sizeof(T));
	}

	T& push(T elem) {
		if (size >= capacity)
			reserve(capacity * 4);

		return (data[size++] = elem);
	}

	T pop() {
		if (!size)
			fatal("Nothing to pop off stack!\n");
		return data[--size];
	}

	T& operator[](u64 idx) {
		#ifdef DEBUG
			if (idx > size)
				error("Stack operator[] out of bounds!\n");
		#endif
		return data[idx];
	}

	~Stack() {
		vmm::free(data);
		data = (T*)0x6767676767676767;
	}
};
