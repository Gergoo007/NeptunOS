#pragma once

#include <ctypes.hh>
#include <cppcompat.hh>

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
struct Opt {
	aligned(alignof(T)) u8 buf[sizeof(T)];
	bool present = false;

	Opt() {  }

	template <typename... Args>
	Opt(Args&&... args): present(true) {
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
		#ifdef DEBUG
		if (!present)
			fatal("option: not present [op  *]\n");
		#endif

		return *(T*)buf;
	}

	constexpr T* operator->() {
		#ifdef DEBUG
		if (!present)
			fatal("option: not present [op ->]\n");
		#endif

		return (T*)buf;
	}

	constexpr T unw() && {
		return move<T>(*(T*)buf);
	}

	~Opt() {
		if (present)
			((T*)buf)->~T();
	}
};

// StackOverflow-ról, elvileg libc++
template<class _Ep>
class initializer_list {
    const _Ep* __begin_;
    size_t    __size_;
    
    inline
    constexpr
    initializer_list(const _Ep* __b, size_t __s) noexcept
        : __begin_(__b),
          __size_(__s)
    {}
public:
    typedef _Ep        value_type;
    typedef const _Ep& reference;
    typedef const _Ep& const_reference;
    typedef size_t    size_type;
    
    typedef const _Ep* iterator;
    typedef const _Ep* const_iterator;
    
    inline
    constexpr
    initializer_list() noexcept : __begin_(nullptr), __size_(0) {}
    
    inline
    constexpr
    size_t    size()  const noexcept {return __size_;}
    
    inline
    constexpr
    const _Ep* begin() const noexcept {return __begin_;}
    
    inline
    constexpr
    const _Ep* end()   const noexcept {return __begin_ + __size_;}
};

template<class _Ep>
inline constexpr const _Ep* begin(initializer_list<_Ep> __il) noexcept {
    return __il.begin();
}

template<class _Ep>
inline constexpr const _Ep* end(initializer_list<_Ep> __il) noexcept {
    return __il.end();
}

template <u64 U, typename T>
struct Array {
	T data[U];

	Array() = default;
	Array(Array& oth) = delete;
	Array(Array&& oth) = delete;
	~Array() = default;
};

// TODO: kompatibilitás Vectorokkal meg Arrayokkal
template <typename T>
struct Slice {
	T* datastart = nullptr;
	u64 datalen = 0;

	Slice(T* data, u64 size): datastart(data), datalen(size) {  }

	constexpr T& operator[](u64 idx) {
		#ifdef DEBUG
		if (idx > datalen)
			fatal("Out of bounds Slice access: %lld vs %lld (data @ %p)\n", idx, datalen, datastart);
		if (!datastart)
			fatal("Slice on nullptr??\n");
		#endif
		return datastart[idx];
	}
};
