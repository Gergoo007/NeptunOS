#pragma once

#include <util/bitmap.hh>
#include <util/string.hh>
#include <util/helpers.hh>
#include <util/smartptrs.hh>
#include <util/allocator.hh>
#include <cppcompat.hh>

static constexpr u64 growfun(u64 cap) {
	if (cap == 0)
		return 8;
	return cap * 2;
}

template <typename T>
struct _generic_iter {
	T* data = nullptr;
	_generic_iter(T* _data): data(_data) {  }
	_generic_iter(_generic_iter& o): data(o.data) {  }
	
	_generic_iter& operator++()		{ data++; return *this; }
	_generic_iter  operator++(int)	{ auto old = *this; data++; return old; }
	
	_generic_iter& operator--()		{ data--; return *this; }
	_generic_iter  operator--(int)	{ auto old = *this; data--; return old; }

	_generic_iter  operator+ (u64 a)	{ return _generic_iter(data + a); }
	_generic_iter  operator- (u64 a)	{ return _generic_iter(data + a); }

	_generic_iter& operator+=(u64 a)	{ data += a; return *this; }
	_generic_iter& operator-=(u64 a)	{ data -= a; return *this; }

	T* operator-(const _generic_iter& o) { return data - o.data; }
	T& operator[](const u64 idx) { return data[idx]; }

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
struct Vector {
	using Iter = _generic_iter<T>;

	T* data = nullptr;
	u64 size = 0;
	u64 capacity = VEC_DEFAULT_SIZE;

	Vector() {
		if (capacity)
			data = (T*)_galloc(capacity * sizeof(T));
	}

	Vector(u64 cap): capacity(cap) {
		if (capacity)
			data = (T*)_galloc(capacity * sizeof(T));
	}

	Vector(std::initializer_list<T> items) {
		capacity = align(items.size(), 8);

		if (capacity) {
			data = (T*)_galloc(capacity * sizeof(T));
			for (const auto& e : items)
				emplace_back(e);
		}
	}

	Vector(const Vector& o) {
		reserve(o.capacity);
		size = o.size;

		for (u64 i = 0; i < o.size; i++)
			new (&data[i]) T(o[i]);
	}

	Vector(Vector&& o) {
		data = o.data;
		size = o.size;
		capacity = o.capacity;

		o.size = 0;
		o.capacity = 0;
		o.data = nullptr;
	}

	Vector& operator=(const Vector& o) {
		for (const auto& e : *this)
			e.~T();

		reserve(o.capacity);
		size = o.size;

		for (u64 i = 0; i < o.size; i++)
			new (&data[i]) T(o[i]);

		return *this;
	}

	Vector& operator=(Vector&& o) {
		for (const auto& e : *this)
			e.~T();

		size = o.size;
		capacity = o.capacity;
		data = o.data;

		o.data = nullptr;
		o.size = 0;
		o.capacity = 0;

		return *this;
	}

	~Vector() {
		for (const auto& e : *this)
			e.~T();

		_gfree(data);
		data = nullptr;
	}

	void reserve(u64 cap) {
		if (cap < size)
			cap = size;

		if (!data && cap) {
			data = (T*)_galloc(cap * sizeof(T));
			capacity = cap;
			return;
		}

		if (!_gtryrealloc(data, cap * sizeof(T))) {
			T* newdata = (T*)_galloc(cap * sizeof(T));
			for (u32 i = 0; i < size; i++)
				new (&newdata[i]) T(move<T>(data[i]));
			for (u32 i = 0; i < size; i++)
				data[i].~T();
			_gfree(data);
			data = newdata;
		}
		capacity = cap;
	}

	T& operator[](const u64 idx) const {
		if constexpr (DBG) {
			if (idx > size)
				fatal("vector access out of bounds! idx %lld size %lld", idx, size);
			if (!data)
				fatal("vector data null (uninitialized)!");
		}
		return data[idx];
	}

	// const T& operator[](const u64 idx) const {
	// 	if constexpr (DBG) {
	// 		if (idx > size)
	// 			fatal("vector access out of bounds! idx %lld size %lld", idx, size);
	// 		if (!data)
	// 			fatal("vector data null (uninitialized)!");
	// 	}
	// 	return data[idx];
	// }

	T& last() {
		return data[size - 1];
	}

	bool operator==(const Vector& o) const {
		if (o.size != size) return false;
		for (u64 i = 0; i < size; i++) {
			if (data[i] != o.data[i])
				return false;
		}
		return true;
	}

	Vector& operator+=(T c) {
		reserve(size + 1);
		data[size++] = c;
		return *this;
	}

	T& push_back(const T& item) {
		if (size >= capacity)
			reserve(growfun(capacity));
		return *(new (&data[size++]) T(item));
	}

	T& push_back(T&& item) {
		if (size >= capacity)
			reserve(growfun(capacity));
		return *(new (&data[size++]) T(move<T>(item)));
	}

	template <typename... Args>
	T& emplace_back(Args&&... args) {
		if (size >= capacity)
			reserve(growfun(capacity));
		if ((size || capacity) && !data)
			fatal("Invalid vector state: data is null but size is %lld (cap %lld)", size, capacity);
		return *(new (&data[size++]) T(forward<Args>(args)...));
	}

	void resize(u64 to) {
		reserve(to);
		if (to < size) {
			while (size != to) {
				data[--size].~T();
			}
		} else if (to > size) {
			while (size != to) {
				new (&data[size++]) T();
			}
		}
	}

	T& pop() {
		if (!size) fatal("Tried to pop an empty stack!");
		return data[--size];
	}

	Iter begin() const { return Iter(data); }
	Iter end() const { return Iter(data + size); }
};

// a size-ba NINCS bele számítva a null terminator
struct String : Vector<char> {
	// Ez mi a faszért kell?
	using Vector<char>::operator+=;

	String(): Vector<char>() {
		if (size)
			data[size] = 0;
	}

	void resize(u64 _size) { Vector::resize(_size); Vector::reserve(_size + 1); data[size] = 0; }

	String(const char* str): Vector<char>(strlen(str)+1) {
		size = capacity - 1;
		memcpy((void*)data, (void*)str, size+1);
		data[size] = 0;
	}

	// Substring
	String(const char* str, u64 len): Vector<char>(len + 1) {
		size = capacity - 1;
		memcpy((void*)data, (void*)str, size);
		data[size] = 0;
	}

	String(std::initializer_list<char> items) {
		capacity = align(items.size(), 8);

		if (capacity) {
			data = (char*)_galloc(capacity * sizeof(char));
			for (const auto& e : items)
				emplace_back(e);
		}

		data[size] = 0;
	}

	String(const String& o): Vector<char>() {
		if (o.size) {
			size = o.size;
			capacity = o.size;
			data = (char*)_galloc(capacity);

			memcpy(data, o.data, size + 1);
		}
	}

	String(String&& o): Vector<char>(o) {  }

	String& operator=(const String& o) {
		if (data) _gfree(data);

		size = o.size;
		capacity = o.size;
		data = (char*)_galloc(capacity);

		if (data)
			memcpy(data, o.data, size + 1);

		return *this;
	}

	String& operator=(String&& o) {
		if (data) _gfree(data);
		data = o.data;
		size = o.size;
		capacity = o.capacity;

		o.data = nullptr;
		o.size = 0;
		o.capacity = 0;

		return *this;
	}

	String& operator+=(const char* s) {
		u64 len = strlen(s);
		reserve(size + len + 1);
		memcpy((void*)&data[size], (void*)s, len);
		size += len;
		data[size] = 0;
		return *this;
	}

	String& operator+=(char c) {
		reserve(size + 1);
		data[size++] = c;
		data[size] = 0;
		return *this;
	}

	char* c_str() const { if (!data) return (char*)""; else return (char*)data; }
};

template <u64 S, typename T>
struct Array {
	using Iter = _generic_iter<T>;
	static constexpr u64 size = S;

	T data[S] = {};

	Array();

	Array(std::initializer_list<T> items) {
		assert(items.__size_ <= size);

		u64 idx = 0;
		for (const auto& e : items)
			data[idx++] = e;
	}

	Array(const Array& o) {
		static_assert(size == o.size);

		for (u64 i = 0; i < o.size; i++)
			data[i].~T();

		for (u64 i = 0; i < o.size; i++)
			new (&data[i]) T(o[i]);
	}


	Array(Array&& o) = default;

	Array& operator=(const Array& o) {
		static_assert(size == o.size);
	
		for (const auto& e : *this)
			e.~T();

		for (u64 i = 0; i < o.size; i++)
			new (&data[i]) T(o[i]);

		return *this;
	}

	~Array() {
		for (auto& e : *this)
			e.~T();
	}

	T& operator[](const u64 idx) {
		if constexpr (DBG) {
			if (idx > S)
				fatal("array<> access out of bounds! idx %lld size %lld", idx, size);
		}
		return data[idx];
	}

	const T& operator[](const u64 idx) const {
		if constexpr (DBG) {
			if (idx > S)
				fatal("array<> access out of bounds! idx %lld size %lld", idx, size);
		}
		return data[idx];
	}

	bool operator==(const Array& o) const {
		if (o.size != size) return false;
		for (u64 i = 0; i < size; i++) {
			if (!(data[i] == o.data[i]))
				return false;
		}
		return true;
	}

	Iter begin() { return Iter(data); }
	Iter end() { return Iter(data + size); }
};

template <typename T>
struct Opt {
	union storage {
		T data;
		bool dummy;

		constexpr storage(): dummy(false) {}
		constexpr storage(const T& d): data(d) {}
		constexpr storage(T&& d): data(d) {}

		constexpr ~storage() {}
	} storage;
	bool present = false;

	constexpr Opt(T&& init): storage(forward<T>(init)), present(true) {  }
	constexpr Opt(const T& init): storage(init), present(true) {  }
	constexpr Opt(): storage(), present(false) {  }

	template <typename... Args>
	constexpr Opt(Args&&... args): storage(T(forward<Args>(args)...)), present(true) {  }

	constexpr T& expect(const char* error = "") {
		if (!present)
			fatal("Optional not present: %s", error);

		return storage.data;
	}

	constexpr T& emplace_back(T&& init) {
		storage.data = T(forward<T>(init));
		present = true;
		return storage.data;
	}

	constexpr ~Opt() {
		if (present)
			storage.data.~T();
	}
};

template <typename T>
struct LinkedList {
	struct Link {
		Link* prev;
		Link* next;
		T data;
		bool last = false;

		Link(Link* _prev, Link* _next, const T& _data): prev(_prev), next(_next), data(_data) {  }
		~Link() { prev = next = nullptr; }
	};

	struct Iter {
		Link* data;
		Iter(Link* _data): data(_data) {  }
		
		Iter& operator++()		{ data=data->last?nullptr:data->next; return *this; }
		Iter  operator++(int)	{ auto old = *this; data=data->last?nullptr:data->next; return old; }
		
		Iter& operator--()		{ data=data->prev; return *this; }
		Iter  operator--(int)	{ auto old = *this; data=data->prev; return old; }

		Iter  operator+ (T* a)	{ fatal("llist: iter operator+ not implemented"); }
		Iter  operator- (T* a)	{ fatal("llist: iter operator-(*) not implemented"); }

		Iter& operator+=(T* a)	{ fatal("llist: iter operator+= not implemented"); }
		Iter& operator-=(T* a)	{ fatal("llist: iter operator-= not implemented"); }

		T* operator-(const Iter& o) { fatal("llist: iter operator-(&) not implemented"); }
		T& operator[](const u64 idx) { fatal("llist: iter operator[] not implemented"); }

		T* operator->()	{ return &data->data; }
		T& operator*()	{ return data->data;  }

		bool operator==(const Iter& o) const { return data == o.data; }
		bool operator!=(const Iter& o) const { return data != o.data; }
		bool operator< (const Iter& o) const { return data  < o.data; }
		bool operator<=(const Iter& o) const { return data <= o.data; }
		bool operator> (const Iter& o) const { return data  > o.data; }
		bool operator>=(const Iter& o) const { return data >= o.data; }
	};

	Link* first = nullptr;
	u64 size = 0;
	MutexSimple m;

	T& push_back(const T& elem) {
		LockguardSimple yes(m);
		size++;
		if (first) {
			Link* newl = new Link(first->prev, first, elem);
			first->prev->next = newl;

			assert(first->prev->last);
			first->prev->last = false;
			
			first->prev = newl;
			newl->last = true;
			return newl->data;
		} else {
			// Create the first link
			first = new Link(nullptr, nullptr, elem);
			first->next = first;
			first->prev = first;
			first->last = true;
			return first->data;
		}
	}

	T& push_front(const T& elem) {
		LockguardSimple yes(m);
		size++;
		if (first) {
			Link* newfirst = new Link(first->prev, first, elem);
			first->prev->next = newfirst;
			first->prev = newfirst;
			first = newfirst;
			return first->data;
		} else {
			// Create the first link
			first = new Link(nullptr, nullptr, elem);
			first->prev = first;
			first->next = first;
			first->last = true;
			return first->data;
		}
	}

	void remove(Link& l) {
		LockguardSimple yes(m);
		if (!size) fatal("Tried to delete elem from empty list (size 0)");
		if (!first) fatal("Tried to delete elem from empty list (first null)");
		size--;

		// Ez az egy elem volt egyedül?
		if (!size) {
			first = nullptr;
		} else {
			l.prev->next = l.next;
			l.next->prev = l.prev;

			if (&l == first)
				first = l.next;
		}

		delete &l;
	}

	void remove(u64 idx) {
		if (idx > size)
			fatal("dlinkedlist: index out of bounds! %lld > %lld", idx, size);

		u64 idx2 = idx;
		Link* l = first;
		while (l && idx2) {
			l = l->next;
			idx2--;
		}
		remove(*l);
	}

	void remove(Iter it) { remove(*it); }

	T& operator[](const u64 idx) {
		u64 idx2 = idx;
		Link* l = first;
		while (l && idx2) {
			l = l->next;
			idx2--;
		}
		if (idx2)
			fatal("dlinkedlist: index out of bounds! %lld", idx);
		return l->data;
	}

	const T& operator[](const u64 idx) const {
		u64 idx2 = idx;
		Link* l = first;
		while (l && idx2) {
			l = l->next;
			idx2--;
		}
		if (idx2)
			fatal("dlinkedlist: index out of bounds! %lld", idx);
		return l->data;
	}

	Link& get_link(u64 idx) {
		u64 idx2 = idx;
		Link* l = first;
		while (l && idx2) {
			l = l->next;
			idx2--;
		}
		if (idx2)
			fatal("dlinkedlist: index out of bounds! %lld", idx);
		return *l;
	}

	Iter begin() { return Iter(first); }
	Iter end() { return Iter(nullptr); }

	~LinkedList() {
		Link* l = first;
		// Különben visszajut a loop a firstre
		if (first)
			first->prev->next = nullptr;
		while (l) {
			Link* next = l->next;
			delete l;
			l = next;
		}
	}
};

constexpr u128 pow(u128 a, u128 b) {
	u128 num = 1;
	while (b--)
		num *= a;
	return num;
}

constexpr u64 hash(u64 key, u64 size) { return key % size; }
constexpr u32 valueperchar = 53;
constexpr u64 hash(const char* val, u64 size) {
	u64 sum = *(val++);
	u64 i = 1;
	while (*val) {
		sum += pow(valueperchar, i++) * (*val);
		val++;
	}
	return sum % size;
}
constexpr u64 hash(const String& val, u64 size) {
	u64 sum = 0;
	for (u64 i = 0; i < val.size; i++)
		sum += pow(valueperchar, i) * val[i];
	return sum % size;
}

template <typename K, typename V>
struct HashMap {
	struct Pair { K key; V value; };

	struct Iter {
		using veciter = typename Vector<LinkedList<Pair>>::Iter;
		using lliter = typename LinkedList<Pair>::Iter;

		veciter vcur, vend;
		lliter lcur;

		void find_next_valid() {
			while (vcur != vend && lcur == vcur->end()) {
				++vcur;
				if (vcur != vend)
					lcur = vcur->begin();
			}
		}

		Iter(veciter _vstart, veciter _vend):
		vcur(_vstart), vend(_vend), lcur(vcur->begin()) {
			if (vcur != vend)
				lcur = vcur->begin();
			find_next_valid();
		}

		Iter& operator++() {
			lcur++;
			find_next_valid();
			return *this;
		}

		Iter operator++(int) {
			Iter i = *this;
			lcur++;
			find_next_valid();
			return i;
		}

		bool operator==(Iter& o) {
			if (vcur == o.vcur && o.vcur == o.vend) return true;
			return vcur == o.vcur && lcur == o.lcur;
		}

		bool operator!=(Iter& o) { return !(*this == o); }

		Pair& operator*() { return *lcur; }
		Pair* operator->() { return &(*lcur); }
	};

	Iter begin() { return Iter(entries.begin(), entries.end()); }
	Iter end() { return Iter(entries.end(), entries.end()); }

	Vector<LinkedList<Pair>> entries;
	static constexpr u32 defsize = 1024;
	u32 size;

	HashMap(): entries(defsize), size(defsize) { entries.resize(size); }
	HashMap(u64 s): entries(s), size(s) { entries.resize(size); }

	// Amikor a kulcs move-olható
	V& operator[](K&& key) {
		auto& bucket = entries[hash(key, size)];
		if (!bucket.size) {
			// Még nincs ilyen elem, be kell szúrni egy alapértelmezett
			// értéket majd visszaadani egy utalást rá
			return bucket.push_front(Pair {
				move<K>(key),
				V()
			}).value;
		} else {
			// Már van ilyen elem, csak meg kell találni a bucketban
			for (auto& e : bucket) {
				if (e.key == key)
					return e.value;
			}
			fatal("Nincs ilyen elem!");
		}
	}

	// Amikor a kulcs nem move-olható
	V& operator[](const K& key) {
		auto& bucket = entries[hash(key, size)];
		if (!bucket.size) {
			// Még nincs ilyen elem, be kell szúrni egy alapértelmezett
			// értéket majd visszaadani egy utalást rá
			return bucket.push_front(Pair {
				key,
				V()
			}).value;
		} else {
			// Már van ilyen elem, csak meg kell találni a bucketban
			for (auto& e : bucket) {
				if (e.key == key)
					return e.value;
			}
			fatal("Nincs ilyen elem!");
		}
	}

	bool has(const K& key) {
		auto& bucket = entries[hash(key, size)];
		for (const auto& e : bucket)
			if (e.key == key)
				return true;
		return false;
	}

	void remove(const K& key) {
		auto& bucket = entries[hash(key, size)];
		if (!bucket.size) fatal("Tried to delete non-existent element!");

		typename LinkedList<Pair>::Link* l = &bucket.get_link(0);
		auto* first = l;
		do {
			if (l->data.key == key) {
				bucket.remove(*l);
				return;
			}
			l = l->next;
		} while (first != l);
		fatal("Element not found!");
	}
};

template <typename T>
struct Span {
	T* first = nullptr;
	u64 size = 0;
	using Iter = _generic_iter<T>;

	constexpr Span() {}
	constexpr Span(T* start, u64 sz): first(start), size(sz) {}
	constexpr Span(std::initializer_list<T> items) = delete("Use vector/array for initializer lists!");

	template <u64 N>
	constexpr Span(T (arr)[N]): first(arr), size(N) {  }

	template <u64 N>
	constexpr Span(const Array<N, T>& arr): first((T*)arr.data), size(N) {  }

	constexpr Span(const Vector<T>& vec): first((T*)vec.data), size(vec.size) {  }

	constexpr Span(const Span& o): first(o.first), size(o.size) {}

	constexpr T& operator[](u64 idx) const {
		if constexpr (DBG)
			assert(idx < size);
		return first[idx];
	}

	constexpr operator T*() { return first; }
	constexpr operator T() { return *first; }

	Iter begin() { return Iter(first); }
	Iter end() { return Iter(first + size); }
};

template <typename T, typename U>
struct Pair {
	T first;
	U second;

	Pair(): first(), second() {  }
	Pair(T f, U s): first(f), second(s) {  }
};
