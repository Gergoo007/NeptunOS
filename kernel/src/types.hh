#pragma once

#include <ctypes.hh>

template <typename T>
constexpr T templatemax(T a) { return a; }
template <typename T>
constexpr const T& templatemax(const T& a, const T& b) { return a>b ? a : b; }
template <typename T, typename... Ts>
constexpr T templatemax(T a, Ts... args) { T b = templatemax(args...); return a > b ? a : b; }

template <typename T>
constexpr T templatemin(T a) { return a; }
template <typename T>
constexpr const T& templatemin(const T& a, const T& b) { return a<b ? a : b; }
template <typename T, typename... Ts>
constexpr T templatemin(T a, Ts... args) { T b = templatemin(args...); return a < b ? a : b; }

// ChatGPT cuz I tried comprehending and got an aneurysm instead
template<typename T, typename U>
struct IsSame {
	static constexpr bool value = false;
};

template<typename T>
struct IsSame<T, T> {
	static constexpr bool value = true;
};

template <typename T, typename... Ts>
struct IndexOf;

// recursive case
template <typename T, typename First, typename... Rest>
struct IndexOf<T, First, Rest...> {
	static constexpr u64 value =
		IsSame<T, First>::value ? 0 : (1 + IndexOf<T, Rest...>::value);
};

// base case (type not found)
// Már csak a T maradt, a Rest... elfogyott
template <typename T>
struct IndexOf<T> {
	static constexpr u64 value = 0;
	static_assert(sizeof(T) != 0, "Type not found in variant alternatives");
};

template <typename T>
struct remove_reference { using type = T; };

template <typename T>
struct remove_reference<T&> { using type = T; };

template <typename T>
struct remove_reference<T&&> { using type = T; };

template <typename T>
constexpr remove_reference<T>::type&&
move(typename remove_reference<T>::type& obj) noexcept {
	return static_cast<typename remove_reference<T>::type&&>(obj);
}

template <typename T>
constexpr T&& forward(typename remove_reference<T>::type& arg) noexcept {
	return static_cast<T&&>(arg);
}

template <typename T>
struct UniquePtr {
	T* ptr = nullptr;

	template <typename... Args>
	UniquePtr(Args&&... args) {
		ptr = new T(forward<Args>(args)...);
	}

	template <typename... Args>
	T& emplace (Args&&... args) {
		if (ptr)
			delete ptr;
		return *(ptr = new T(forward<Args>(args)...));
	}

	constexpr T& operator*() {
		if (ptr)
			return *ptr;
		else
			fatal("uptr: not present [op  *]\n");
	}

	constexpr T* operator->() {
		if (ptr)
			return ptr;
		else
			fatal("uptr: not present [op ->]\n");
	}

	UniquePtr(UniquePtr& other) = delete;
	UniquePtr(UniquePtr&& other) {
		ptr = other.ptr;
		other.ptr = nullptr;
	}

	~UniquePtr() {
		if (ptr)
			delete ptr;
	}
};

template <typename T>
struct Option {
	aligned(alignof(T)) u8 buf[sizeof(T)];
	bool present = false;

	Option() {  }

	template <typename... Args>
	Option(Args&&... args): present(true) {
		new ((T*)buf) T(forward<Args>(args)...);
	}

	template <typename... Args>
	void emplace(Args&&... args) {
		if (present)
			((T*)buf)->~T();

		present = true;
		new ((T*)buf) T(forward<Args>(args)...);
	}

	constexpr T& operator*() {
		if (present)
			return *(T*)buf;
		else
			fatal("option: not present [op  *]\n");
	}

	constexpr T* operator->() {
		if (present)
			return (T*)buf;
		else
			fatal("option: not present [op ->]\n");
	}

	~Option() {
		if (present)
			((T*)buf)->~T();
	}
};
