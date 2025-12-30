#pragma once

template <typename T>
struct unique_ptr {
	T* ptr = nullptr;

	unique_ptr() = delete;
	unique_ptr(unique_ptr& o) = delete;
	unique_ptr& operator=(unique_ptr& o) = delete;
	unique_ptr& operator=(T* o) = delete;

	unique_ptr(unique_ptr&& o) noexcept {
		ptr = o.ptr;
		o.ptr = nullptr;
	}

	unique_ptr& operator=(unique_ptr&& o) noexcept {
		if (ptr) delete ptr;

		ptr = o.ptr;
		o.ptr = nullptr;

		return *this;
	}

	unique_ptr(T* p): ptr(p) {  }
	unique_ptr(T&& t) { ptr = new T(move<T>(t)); }

	template <typename... Args>
	unique_ptr(Args&&... args) { ptr = new T(forward<Args>(args)...); }

	T& operator*() { return *ptr; }
	T* operator->() { return ptr; }

	~unique_ptr() { delete ptr; }
};
