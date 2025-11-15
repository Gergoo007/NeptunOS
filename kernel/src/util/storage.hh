#pragma once

#include <mm/vmm.hh>
#include <util/bitmap.hh>
#include <util/string.hh>
#include <util/helpers.hh>
#include <cppcompat.hh>

// itt lehet mókolni, a firefox elvikeg cap*cap-et használ
static constexpr u64 growfun(u64 cap) {
	return cap * 2;
}

template <typename T>
struct _generic_iter {
	T* data;
	_generic_iter(T* _data): data(_data) {  }
	
	_generic_iter& operator++()		{ data++; return *this; }
	_generic_iter  operator++(int)	{ auto old = *this; data++; return old; }
	
	_generic_iter& operator--()		{ data--; return *this; }
	_generic_iter  operator--(int)	{ auto old = *this; data--; return old; }

	_generic_iter  operator+ (T* a)	{ return _generic_iter(data + a); }
	_generic_iter  operator- (T* a)	{ return _generic_iter(data + a); }

	_generic_iter& operator+=(T* a)	{ data += a; return *this; }
	_generic_iter& operator-=(T* a)	{ data -= a; return *this; }

	T* operator-(const _generic_iter& o) { return data - o.data; }
	T& operator[](u64 idx) { return data[idx]; }

	T* operator->()	{ return data; }
	T& operator*()	{ return *data; }

	bool operator==(const _generic_iter& o) const { return data == o.data; }
    bool operator!=(const _generic_iter& o) const { return data != o.data; }
    bool operator< (const _generic_iter& o) const { return data  < o.data; }
    bool operator<=(const _generic_iter& o) const { return data <= o.data; }
    bool operator> (const _generic_iter& o) const { return data  > o.data; }
    bool operator>=(const _generic_iter& o) const { return data >= o.data; }
};

template <typename T>
_generic_iter(T*) -> _generic_iter<T>;

template <typename T>
struct vector {
	using iter = _generic_iter<T>;

	T* data;
	u64 size;
	u64 capacity;

	vector() {
		capacity = default_vec_size;
		size = 0;
		data = (T*)kmalloc(capacity * sizeof(T));
	}

	vector(u64 cap): capacity(cap) {
		size = 0;
		data = (T*)kmalloc(capacity * sizeof(T));
	}

	vector(std::initializer_list<T> items) {
		capacity = align(items.size(), 8);
		size = 0;
		data = (T*)kmalloc(capacity * sizeof(T));
		for (const auto& e : items)
			emplace(e);
	}

	vector(vector& o) {
		capacity = 0;
		data = nullptr;
		if (capacity < o.capacity)
			reserve(o.capacity);

		size = o.size;

		for (u64 i = 0; i < o.size; i++)
			data[i].~T();

		for (u64 i = 0; i < o.size; i++)
			data[i] = o[i];
	}


	vector(vector&& o) {
		data = o.data;
		size = o.size;
		capacity = o.capacity;

		o.data = nullptr;
		o.size = 0;
		o.capacity = 0;
		kfree(o.data);
		o.data = nullptr;
	}

	vector& operator=(vector& o) {
		for (const auto& e : *this)
			e.~T();

		reserve(o.capacity);
		size = o.size;

		for (u64 i = 0; i < o.size; i++)
			data[i](o[i]);

		return *this;
	}

	~vector() {
		for (const auto& e : *this)
			e.~T();

		kfree(data);
		data = (T*)0x6767676767676767;
	}

	void reserve(u64 cap) {
		capacity = cap;
		data = (T*)krealloc((void*)data, capacity * sizeof(T));
	}

	T& operator[](u64 idx) {
		if constexpr (debug) {
			if (!data)
				fatal("vector data null (was it moved?)!");
			if (idx > size)
				fatal("vector access out of bounds! idx %lld size %lld", idx, size);
		}
		return data[idx];
	}

	T& push_back(T item) {
		if (size == capacity)
			reserve(growfun(capacity));
		data[size++] = item;
	}

	template <typename... Args>
	T& emplace(Args&&... args) {
		if (size == capacity)
			reserve(growfun(capacity));
		return *(new (&data[size++]) T(forward<Args>(args)...));
	}

	const iter begin() { return iter(data); }
	const iter end() { return iter(data + size); }
};

// a size-ba NINCS bele számítva a null terminator
struct string : vector<char> {
	string(const char* str) {
		size = strlen(str);
		reserve(size);
		memcpy((void*)data, (void*)str, size+1);
	}

	char* c_str() const {
		return (char*)data;
	}
};

template <typename T>
struct optional {
	optional(T init) {  }

	template <typename... Args>
	optional(Args&&... args) {}

	template <typename... Args>
	T& emplace() {}
};
