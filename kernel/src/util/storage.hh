#pragma once

#include <mm/vmm.hh>
#include <util/bitmap.hh>
#include <util/string.hh>
#include <util/helpers.hh>
#include <util/smartptrs.hh>
#include <cppcompat.hh>

// itt lehet mókolni, a firefox elvikeg cap*cap-et használ
static constexpr u64 growfun(u64 cap) {
	if (cap == 0)
		return 8;
	return cap + 10;
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
struct vector {
	using iter = _generic_iter<T>;

	T* data = nullptr;
	u64 size = 0;
	u64 capacity = VEC_DEFAULT_SIZE;

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
				emplace_back(e);
		}
	}

	vector(const vector& o) {
		reserve(o.capacity);
		size = o.size;

		for (u64 i = 0; i < o.size; i++)
			new (&data[i]) T(o[i]);
	}

	vector(vector&& o) {
		data = o.data;
		size = o.size;
		capacity = o.capacity;

		o.size = 0;
		o.capacity = 0;
		o.data = nullptr;
	}

	vector& operator=(const vector& o) {
		for (const auto& e : *this)
			e.~T();

		reserve(o.capacity);
		size = o.size;

		for (u64 i = 0; i < o.size; i++)
			new (&data[i]) T(o[i]);

		return *this;
	}

	vector& operator=(vector&& o) {
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

	~vector() {
		for (const auto& e : *this)
			e.~T();

		kfree(data);
		data = nullptr;
	}

	void reserve(u64 cap) {
		if (cap < size)
			cap = size;

		if (!data && cap) {
			data = (T*)kmalloc(cap * sizeof(T));
			capacity = cap;
			return;
		}

		if (!vmm_try_realloc(data, cap * sizeof(T), __FILE__, __LINE__)) {
			T* newdata = (T*)kmalloc(cap * sizeof(T));
			for (u32 i = 0; i < size; i++)
				new (&newdata[i]) T(move<T>(data[i]));
			for (u32 i = 0; i < size; i++)
				data[i].~T();
			kfree(data);
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

	bool operator==(const vector& o) const {
		if (o.size != size) return false;
		for (u64 i = 0; i < size; i++) {
			if (data[i] != o.data[i])
				return false;
		}
		return true;
	}

	vector& operator+=(T c) {
		reserve(size + 1);
		data[size++] = c;
		return *this;
	}

	T& push_back(const T& item) {
		if (size >= capacity)
			reserve(growfun(capacity));
		// return *(new (&data[size++]) T(item));
		return *(new (&data[size++]) T(item));
	}

	T& push_back(T&& item) {
		if (size >= capacity)
			reserve(growfun(capacity));
		// return *(new (&data[size++]) T(item));
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

	iter begin() const { return iter(data); }
	iter end() const { return iter(data + size); }
};

// a size-ba NINCS bele számítva a null terminator
struct string : vector<char> {
	// Ez mi a faszért kell?
	using vector<char>::operator+=;

	string() = default;

	void resize(u64 _size) { vector::resize(_size); vector::reserve(_size + 1); data[size] = 0; }

	string(const char* str): vector<char>(strlen(str)+1) {
		size = capacity - 1;
		memcpy((void*)data, (void*)str, size+1);
		data[size] = 0;
	}

	// Substring
	string(const char* str, u64 len): vector<char>(len + 1) {
		size = capacity - 1;
		memcpy((void*)data, (void*)str, size);
		data[size] = 0;
	}

	string(std::initializer_list<char> items) {
		capacity = align(items.size(), 8);

		if (capacity) {
			data = (char*)kmalloc(capacity * sizeof(char));
			for (const auto& e : items)
				emplace_back(e);
		}

		data[size] = 0;
	}

	string(const string& o): vector<char>() {
		if (o.size) {
			size = o.size;
			capacity = o.size;
			data = (char*)kmalloc(capacity);

			memcpy(data, o.data, size + 1);
		}
	}

	string(string&& o): vector<char>(o) {  }

	string& operator=(const string& o) {
		if (data) kfree(data);

		size = o.size;
		capacity = o.size;
		data = (char*)kmalloc(capacity);

		if (data)
			memcpy(data, o.data, size + 1);

		return *this;
	}

	string& operator=(string&& o) {
		if (data) kfree(data);
		data = o.data;
		size = o.size;
		capacity = o.capacity;

		o.data = nullptr;
		o.size = 0;
		o.capacity = 0;

		return *this;
	}

	string& operator+=(const char* s) {
		u64 len = strlen(s);
		reserve(size + len + 1);
		memcpy((void*)&data[size], (void*)s, len);
		size += len;
		data[size] = 0;
		return *this;
	}

	string& operator+=(char c) {
		reserve(size + 1);
		data[size++] = c;
		data[size] = 0;
		return *this;
	}

	char* c_str() const { if (!data) return (char*)""; else return (char*)data; }
};

struct stringv {
	using iter = _generic_iter<const char>;

	const char* s = nullptr;
	stringv() = delete("14");
	stringv(const char* d): s(d) {  }

	const char& operator[](u64 idx) { return s[idx]; }

	bool operator==(const char* o) { return !strcmp(s, o); }
	bool operator!=(const char* o) { return strcmp(s, o); }

	iter begin() { return iter(s); }
	iter end() { u64 len = strlen(s); return iter(s + len); }
};

template <u64 S, typename T>
struct array {
	using iter = _generic_iter<T>;
	static constexpr u64 size = S;

	T data[S] = {};

	array();

	array(std::initializer_list<T> items) {
		assert(items.__size_ <= size);

		u64 idx = 0;
		for (const auto& e : items)
			data[idx++] = e;
	}

	array(const array& o) {
		static_assert(size == o.size);

		for (u64 i = 0; i < o.size; i++)
			data[i].~T();

		for (u64 i = 0; i < o.size; i++)
			new (&data[i]) T(o[i]);
	}


	array(array&& o) = default;

	array& operator=(const array& o) {
		static_assert(size == o.size);
	
		for (const auto& e : *this)
			e.~T();

		for (u64 i = 0; i < o.size; i++)
			new (&data[i]) T(o[i]);

		return *this;
	}

	~array() {
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

	bool operator==(const array& o) const {
		if (o.size != size) return false;
		for (u64 i = 0; i < size; i++) {
			if (!(data[i] == o.data[i]))
				return false;
		}
		return true;
	}

	iter begin() { return iter(data); }
	iter end() { return iter(data + size); }
};

template <typename T>
struct optional {
	__attribute__((aligned(alignof(T))))
	u8 storage[sizeof(T)];
	bool present = false;

	optional(T&& init): present(true) { new (&storage) T(forward<T>(init)); }
	optional(const T& init): present(true) { new (&storage) T(init); }
	optional(): present(false) {  }

	template <typename... Args>
	optional(Args&&... args): present(true) { new (&storage) T(forward<Args>(args)...); }

	T& expect(const char* error) {
		if (!present)
			fatal("Optional not present: %s", error);

		return *(T*)storage;
	}

	T& emplace_back(T&& init) {
		new (&storage) T(forward<T>(init));
		present = true;
		return *(T*)storage;
	}
};

template <typename T>
struct llist {
	struct link_t {
		link_t* prev;
		link_t* next;
		T data;
		bool last = false;

		link_t(link_t* _prev, link_t* _next, const T& _data): prev(_prev), next(_next), data(_data) {  }
		~link_t() { prev = next = nullptr; }
	};

	struct iter {
		link_t* data;
		iter(link_t* _data): data(_data) {  }
		
		iter& operator++()		{ data=data->last?nullptr:data->next; return *this; }
		iter  operator++(int)	{ auto old = *this; data=data->last?nullptr:data->next; return old; }
		
		iter& operator--()		{ data=data->prev; return *this; }
		iter  operator--(int)	{ auto old = *this; data=data->prev; return old; }

		iter  operator+ (T* a)	{ fatal("llist: iter operator+ not implemented"); }
		iter  operator- (T* a)	{ fatal("llist: iter operator-(*) not implemented"); }

		iter& operator+=(T* a)	{ fatal("llist: iter operator+= not implemented"); }
		iter& operator-=(T* a)	{ fatal("llist: iter operator-= not implemented"); }

		T* operator-(const iter& o) { fatal("llist: iter operator-(&) not implemented"); }
		T& operator[](const u64 idx) { fatal("llist: iter operator[] not implemented"); }

		T* operator->()	{ return &data->data; }
		T& operator*()	{ return data->data;  }

		bool operator==(const iter& o) const { return data == o.data; }
		bool operator!=(const iter& o) const { return data != o.data; }
		bool operator< (const iter& o) const { return data  < o.data; }
		bool operator<=(const iter& o) const { return data <= o.data; }
		bool operator> (const iter& o) const { return data  > o.data; }
		bool operator>=(const iter& o) const { return data >= o.data; }
	};

	link_t* first = nullptr;
	u64 size = 0;
	mutex m;

	T& push_back(const T& elem) {
		lockguard yes(m);
		size++;
		if (first) {
			link_t* newl = new link_t(first->prev, first, elem);
			first->prev->next = newl;

			assert(first->prev->last);
			first->prev->last = false;
			
			first->prev = newl;
			newl->last = true;
			return newl->data;
		} else {
			// Create the first link
			first = new link_t(nullptr, nullptr, elem);
			first->next = first;
			first->prev = first;
			first->last = true;
			return first->data;
		}
	}

	T& push_front(const T& elem) {
		lockguard yes(m);
		size++;
		if (first) {
			link_t* newfirst = new link_t(first->prev, first, elem);
			warn("%p", first->prev);
			first->prev->next = newfirst;
			first->prev = newfirst;
			first = newfirst;
			return first->data;
		} else {
			// Create the first link
			first = new link_t(nullptr, nullptr, elem);
			first->prev = first;
			first->next = first;
			first->last = true;
			return first->data;
		}
	}

	void remove(link_t& l) {
		lockguard yes(m);
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
		link_t* l = first;
		while (l && idx2) {
			l = l->next;
			idx2--;
		}
		remove(*l);
	}

	void remove(iter it) { remove(*it); }

	T& operator[](const u64 idx) {
		u64 idx2 = idx;
		link_t* l = first;
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
		link_t* l = first;
		while (l && idx2) {
			l = l->next;
			idx2--;
		}
		if (idx2)
			fatal("dlinkedlist: index out of bounds! %lld", idx);
		return l->data;
	}

	link_t& get_link(u64 idx) {
		u64 idx2 = idx;
		link_t* l = first;
		while (l && idx2) {
			l = l->next;
			idx2--;
		}
		if (idx2)
			fatal("dlinkedlist: index out of bounds! %lld", idx);
		return *l;
	}

	iter begin() { return iter(first); }
	iter end() { return iter(nullptr); }

	~llist() {
		link_t* l = first;
		// Különben visszajut a loop a firstre
		if (first)
			first->prev->next = nullptr;
		while (l) {
			link_t* next = l->next;
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
constexpr u64 hash(const string& val, u64 size) {
	u64 sum = 0;
	for (u64 i = 0; i < val.size; i++)
		sum += pow(valueperchar, i) * val[i];
	return sum % size;
}

template <typename K, typename V>
struct hashmap {
	struct pair_t { K key; V value; };

	struct iter {
		using veciter = vector<llist<pair_t>>::iter;
		using lliter = llist<pair_t>::iter;

		veciter vcur, vend;
		lliter lcur;

		void find_next_valid() {
			while (vcur != vend && lcur == vcur->end()) {
				++vcur;
				if (vcur != vend)
					lcur = vcur->begin();
			}
		}

		iter(veciter _vstart, veciter _vend):
		vcur(_vstart), vend(_vend), lcur(vcur->begin()) {
			if (vcur != vend)
				lcur = vcur->begin();
			find_next_valid();
		}

		iter& operator++() {
			lcur++;
			find_next_valid();
			return *this;
		}

		iter operator++(int) {
			iter i = *this;
			lcur++;
			find_next_valid();
			return i;
		}

		bool operator==(iter& o) {
			if (vcur == o.vcur && o.vcur == o.vend) return true;
			return vcur == o.vcur && lcur == o.lcur;
		}

		bool operator!=(iter& o) { return !(*this == o); }

		pair_t& operator*() { return *lcur; }
		pair_t* operator->() { return &(*lcur); }
	};

	iter begin() { return iter(entries.begin(), entries.end()); }
	iter end() { return iter(entries.end(), entries.end()); }

	vector<llist<pair_t>> entries;
	static constexpr u32 defsize = 1024;
	u32 size;

	hashmap(): entries(defsize), size(defsize) { entries.resize(size); }
	hashmap(u64 s): entries(s), size(s) { entries.resize(size); }

	// Amikor a kulcs move-olható
	V& operator[](K&& key) {
		auto& bucket = entries[hash(key, size)];
		if (!bucket.size) {
			// Még nincs ilyen elem, be kell szúrni egy alapértelmezett
			// értéket majd visszaadani egy utalást rá
			return bucket.push_front(pair_t {
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
			return bucket.push_front(pair_t {
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

		typename llist<pair_t>::link_t* l = &bucket.get_link(0);
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
struct span {
	T* first = nullptr;
	u64 size = 0;
	using iter = _generic_iter<T>;

	constexpr span() {}
	constexpr span(T* start, u64 sz): first(start), size(sz) {}
	constexpr span(std::initializer_list<T> items) = delete("Use vector/array for initializer lists!");

	template <u64 N>
	constexpr span(T (arr)[N]): first(arr), size(N) {  }

	template <u64 N>
	constexpr span(const array<N, T>& arr): first((T*)arr.data), size(N) {  }

	constexpr span(const vector<T>& vec): first((T*)vec.data), size(vec.size) {  }

	constexpr span(const span& o): first(o.first), size(o.size) {}

	constexpr T& operator[](u64 idx) const {
		if constexpr (DBG)
			assert(idx < size);
		return first[idx];
	}

	iter begin() { return iter(first); }
	iter end() { return iter(first + size); }
};

template <typename T, typename U>
struct pair {
	T first;
	U second;

	pair(): first(), second() {  }
	pair(T f, U s): first(f), second(s) {  }
};
