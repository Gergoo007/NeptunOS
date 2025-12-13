#pragma once

#include <mm/vmm.hh>
#include <util/bitmap.hh>
#include <util/string.hh>
#include <util/helpers.hh>
#include <util/stacktrace.hh>
#include <cppcompat.hh>

// itt lehet mókolni, a firefox elvikeg cap*cap-et használ
static constexpr u64 growfun(u64 cap) {
	if (cap == 0)
		return 8;
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

	T* data = nullptr;
	u64 size = 0;
	u64 capacity = default_vec_size;

	vector() {
		if (capacity)
			data = (T*)kmalloc(capacity * sizeof(T));
	}

	vector(u64 cap): capacity(cap) {
		if (capacity)
			data = (T*)kmalloc(capacity * sizeof(T));
	}

	vector(std::initializer_list<T> items) {
		capacity = align(items.size(), 8);

		if (capacity) {
			data = (T*)kmalloc(capacity * sizeof(T));
			for (const auto& e : items)
				emplace(e);
		}
	}

	vector(vector& o) {
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

		if (o.data)
			kfree(o.data);
		o.size = 0;
		o.capacity = 0;
		o.data = nullptr;
	}

	vector& operator=(vector& o) {
		for (const auto& e : *this)
			e.~T();

		reserve(o.capacity);
		size = o.size;

		for (u64 i = 0; i < o.size; i++)
			data[i] = T(o[i]);

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
		if (cap == 0)
			data = nullptr;
		else
			data = (T*)krealloc((void*)data, capacity * sizeof(T));
	}

	T& operator[](u64 idx) {
		if constexpr (debug) {
			if (idx > size)
				fatal("vector access out of bounds! idx %lld size %lld", idx, size);
			if (!data)
				fatal("vector data null (uninitialized)!");
		}
		return data[idx];
	}

	T& last() const {
		return data[size - 1];
	}

	bool operator==(const vector& o) const {
		if (o.size != size) return false;
		for (u64 i = 0; i < size; i++) {
			if (data[i] != o.data[i])
				return false;
		}
		return true;
	}

	T& push_back(T item) {
		if (size >= capacity)
			reserve(growfun(capacity));
		return data[size++] = item;
	}

	template <typename... Args>
	T& emplace(Args&&... args) {
		if (size >= capacity)
			reserve(growfun(capacity));
		return *(new (&data[size++]) T(forward<Args>(args)...));
	}

	const iter begin() const { return iter(data); }
	const iter end() const { return iter(data + size); }
};

// a size-ba NINCS bele számítva a null terminator
struct string : vector<char> {
	string(const char* str): vector<char>(strlen(str)+1) {
		size = strlen(str);
		memcpy((void*)data, (void*)str, size+1);
	}

	char* c_str() const {
		return (char*)data;
	}
};

template <u32 S, typename T>
struct array {
	using iter = _generic_iter<T>;
	static constexpr u32 size = S;

	T data[S] = {};

	array();

	array(std::initializer_list<T> items) {
		assert(items.__size_ <= size);

		u32 idx = 0;
		for (const auto& e : items)
			data[idx++] = e;
	}

	array(array& o) {
		static_assert(size == o.size);

		for (u64 i = 0; i < o.size; i++)
			data[i].~T();

		for (u64 i = 0; i < o.size; i++)
			data[i] = T(o[i]);
	}


	array(array&& o) = default;

	array& operator=(array& o) {
		static_assert(size == o.size);
	
		for (const auto& e : *this)
			e.~T();

		for (u64 i = 0; i < o.size; i++)
			data[i] = T(o[i]);

		return *this;
	}

	~array() {
		for (auto& e : *this)
			e.~T();
	}

	T& operator[](u64 idx) {
		if constexpr (debug) {
			if (idx > S)
				fatal("array<> access out of bounds! idx %lld size %lld", idx, size);
		}
		return data[idx];
	}

	bool operator==(array& o) {
		if (o.size != size) return false;
		for (u64 i = 0; i < size; i++) {
			if (!(data[i] == o.data[i]))
				return false;
		}
		return true;
	}

	const iter begin() { return iter(data); }
	const iter end() { return iter(data + size); }
};

template <typename T>
struct optional {
	optional(T init) {  }

	template <typename... Args>
	optional(Args&&... args) {}

	template <typename... Args>
	T& emplace() {}
};
