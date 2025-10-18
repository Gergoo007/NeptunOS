#pragma once

#include <mm/vmm.hh>
#include <util/bitmap.hh>
#include <util/string.hh>
#include <cppcompat.hh>

namespace acpi { struct Name; }

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
	u64 capacity = 0;
	u64 size = 0;

	Vector() {  }

	Vector(u64 cap): capacity(align(cap, 16)) {
		data = (T*)kmalloc(capacity * sizeof(T));
	}

private:
	void _copy(Vector& other) {
		size = other.size;
		capacity = other.capacity;
		data = (T*)kmalloc(capacity * sizeof(T));
		// memcpy(data, other.data, other.size);
		for (u64 i = 0; i < size; i++)
			new (data + i) T(other.data[i]);
	}

public:
	Vector(const Vector& other) {
		_copy((Vector&)other);
	};

	Vector(Vector& other) {
		_copy((Vector&)other);
	}

	Vector(Vector&& other) {
		size = other.size;
		capacity = other.capacity;
		data = other.data;
		other.data = nullptr;
		other.size = 0;
		other.capacity = 0;
	}

	Vector(T* from, u64 bytes): capacity(align(bytes, 16)) {
		data = (T*)kmalloc(capacity * sizeof(T));
		memcpy(data, from, bytes);
		size = bytes;
	}

	void reserve(u64 cap) {
		capacity = cap;
		data = (T*)krealloc(data, capacity * sizeof(T));
	}

	void reserve() {
		capacity = capacity ? capacity*4 : sizeof(T) * 8;
		data = (T*)krealloc(data, capacity * sizeof(T));
	}

	T& operator[](i64 idx) {
		#ifdef DEBUG
		if ((idx < 0 && -idx > size) || (idx > 0 && idx >= size))
			error("Vector (%p) out of bounds (idx %x)!\n", this, idx);
		#endif
		if (idx > 0 || idx == 0)
			return data[idx];
		else
			return data[size - idx];
	}

	bool operator==(Vector<T>& other) const {
		if (other.data == data) return true;
		if (other.size != size) return false;

		return !memcmp(data, other.data, size);
	}

	bool operator!=(Vector<T>& other) const { return !this->operator==(other); }

	bool operator==(const Vector<T>& other) const {
		if (other.data == data) return true;
		if (other.size != size) return false;

		return !memcmp(data, other.data, size);
	}

	bool operator!=(const Vector<T>& other) const { return !this->operator==(other); }

	template <typename... Args>
	T& emplace(Args&&... args) {
		if (size >= capacity)
			reserve();
		return *( new ((void*)(data + size++)) T(forward<Args>(args)...) );
	}

	Iterator begin() const {
		return Iterator(data);
	}

	Iterator end() const {
		return Iterator(data + size);
	}

	~Vector() {
		for (u64 i = 0; i < size; i++)
			data[i].~T();

		kfree(data);
		// M A S O N
		data = (T*)0x6767676767676767;
	}
};

struct String : Vector<char> {
	String(): Vector<char>(8) {  }
	String(u64 _size): Vector<char>(_size) {  }
	String(const char* s): Vector<char>(strlen(s) + 1) {
		u32 len = strlen(s);
		memcpy((void*)data, (void*)s, len);
		size = len;
		data[size] = 0;
	}
	String(const char* s, u64 strsize): Vector<char>(strsize + 1) {
		memcpy((void*)data, (void*)s, strsize);
		size = strsize;
		data[size] = 0;
	}
	String(const String& str): Vector<char>(str) { data[size] = 0; }
	String& operator=(const String& to) {
		if (data)
			kfree(data);
		size = to.capacity;
		size = to.size;
		data = (char*)kmalloc(capacity);
		memcpy(data, to.data, size);
		data[size] = 0;
		return *this;
	}
	String& operator=(const char* s) {
		size = strlen(s);
		capacity = align(size, 16);
		if (data)
			kfree(data);
		data = (char*)kmalloc(capacity);
		memcpy(data, (void*)s, size);
		data[size] = 0;
		return *this;
	}
	String operator+(const String& other) {
		String ret = other;
		ret.reserve(ret.size + size + 1);
		memcpy(ret.data + ret.size, data, size);
		ret.size += size;
		data[size] = 0;
		return ret;
	}
	String& operator+=(const String& other) {
		reserve(other.size + size + 1);
		memcpy(data + size, other.data, other.size);
		size += other.size;
		data[size] = 0;
		return *this;
	}
	String operator+(const char* other) {
		return String(other) + *this;
	}
	String& operator+=(const char* other) {
		u32 othsize = strlen(other);
		reserve(othsize + size + 1);
		memcpy(data + size, (void*)other, othsize);
		size += othsize;
		data[size] = 0;
		return *this;
	}
	String& operator+=(const char c) {
		reserve(capacity + 1);
		data[size++] = c;
		data[size] = 0;
		return *this;
	}

	inline constexpr char* c_str() const { return data; }
};

template <typename T>
struct Stack : Vector<T> {
	T& push(T elem) {
		if (this->size >= this->capacity)
			this->reserve(this->capacity * 4);

		return (this->data[this->size++] = elem);
	}

	T& last() const { return this->data[this->size-1]; }

	T pop() {
		if (!this->size)
			fatal("Nothing to pop off stack!\n");
		return this->data[--this->size];
	}
};

template <typename... Ts>
struct Variant {
	static constexpr u64 maxsize = templatemax(sizeof(Ts)...);
	u8 storage[maxsize];
	u64 idx = -1;

	u64 which() const { return idx; }

	template <typename T, typename... Args>
	T& emplace(Args&&... args) {
		destruct_current();

		idx = IndexOf<T, Ts...>::value;
		if (idx >= sizeof...(Ts))
			fatal("A tipus nincs benne a listaba!\n");
		return *(new (storage) T(forward<Args>(args)...));
	}

	Variant() {  }

	template <typename T>
	T& get() const {
		if (idx != IndexOf<T, Ts...>::value)
			fatal("Nem ez a tipus az aktiv (%d; aktiv: %d)!\n", IndexOf<T, Ts...>::value, idx);
		return *(T*)storage;
	}

	void destruct_current() {
		if (idx == -1)
			return;

		u64 i = 0;
		((idx == i++ ? ( ((Ts*)storage)->~Ts() ) : void(0)), ...);

		idx = -1;
	}

	~Variant() {
		destruct_current();
	}
};
