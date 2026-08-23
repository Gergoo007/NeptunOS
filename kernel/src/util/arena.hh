#pragma once

#include <util/heapbitmap.hh>

template <typename T>
struct Arena {
	struct Handle  {
		// u32 generation;
		u32 index;
	};

	struct Iterator {
		Arena* arena;
		u64 idx;

		Iterator& operator++() {
			idx++;
			while (idx < arena->data.capacity && arena->free_slots.get(idx) != true) {
				idx++;
			}
			return *this;
		}

		T& operator*() {
			return arena->data.data[idx];
		}

		bool operator==(const Iterator& rhs) const {
			return rhs.idx == idx;
		}
	};

	Vector<T> data;
	// Vector<u32> slots; // keeps track of gens
	HeapBitmap free_slots; // keeps track of used up slots

	Arena(): data(0), free_slots(0) {  }
	Arena(u64 cap): data(cap), free_slots(cap) {  }
	Arena(std::initializer_list<T> init): data(init.size()), free_slots(init.size()) {
		for (const auto& item : init) {
			push_back(item);
		}
	}

	void resize(u64 newsize) {
		data.reserve(newsize);
		free_slots.resize(newsize);
	}

	Handle push_back(const T& item) {
		u64 idx = free_slots.find_and_set();
		if (idx == -1ull) {
			u64 newsize = growfun(data.capacity);
			resize(newsize);
			idx = free_slots.find_and_set();
		}
		new (&data.data[idx]) T(item);
		return Handle { idx };
	}

	template <typename... Args>
	Handle emplace_back(Args&&... args) {
		u64 idx = free_slots.find_and_set();
		if (idx == -1ull) {
			u64 newsize = data.capacity * 4;
			resize(newsize);
			idx = free_slots.find_and_set();
		}
		new (&data.data[idx]) T(forward<Args>(args)...);
		return Handle { idx };
	}

	void remove(Handle handle) {
		data.data[handle.index].~T();
		free_slots.set(handle.index, false);
	}

	T& operator[](Handle idx) {
		return data.data[idx.index];
	}

	Iterator begin() {
		u64 first_used = 0;
		while (free_slots.get(first_used) != true) {
			first_used++;
		}
		return Iterator(this, first_used);
	}

	Iterator end() {
		return Iterator(this, data.capacity);
	}
};

template <typename T>
struct GenerationalArena {
	struct Handle  {
		u32 generation;
		u32 index;
	};

	Vector<T> data;
	Vector<u32> slots; // keeps track of gens
	HeapBitmap free_slots; // keeps track of used up slots

	GenerationalArena() = default;
	GenerationalArena(u64 cap): data(cap), slots(cap) {  }

	void resize(u64 newsize) {
		data.resize(newsize);
		slots.resize(newsize);
		free_slots.resize(newsize);
	}
};
