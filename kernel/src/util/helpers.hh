#pragma once

using u64 = unsigned long long;
using size_t = __SIZE_TYPE__;

template <typename T>
struct remove_const			{ using type = T; };
template <typename T>
struct remove_const<const T>{ using type = T; };

template <typename T>
struct remove_volatile			{ using type = T; };
template <typename T>
struct remove_volatile<volatile T>{ using type = T; };

template <typename T>
struct remove_reference		{ using type = T; };
template <typename T>
struct remove_reference<T&>	{ using type = T; };
template <typename T>
struct remove_reference<T&&>{ using type = T; };

template <typename T>
struct remove_ptr			{ using type = T; };
template <typename T>
struct remove_ptr<T*>		{ using type = T; };

template <typename T, typename U>
struct is_same				{ static constexpr bool value = false; };
template <typename T>
struct is_same<T, T>		{ static constexpr bool value = true; };

// template <typename T, typename... Ts>
// struct index_of;

// template <typename T, typename First, typename... Rest>
// struct index_of<T, First, Rest...> {
// 	static constexpr u64 value =
// 		is_same<T, First>::value ? 0 : (1 + index_of<T, Rest...>::value);
// };

// template <typename T>
// struct index_of<T> {
// 	static constexpr u64 value = 0;
// 	static_assert(sizeof(T) != 0, "Type not found in index_of Ts...");
// };

template <typename T, typename... Ts>
struct index_of0;

template <typename T, typename First, typename... Rest>
struct index_of0<T, First, Rest...> {
	static constexpr u64 value =
		is_same<T, First>::value ? 0 : (1 + index_of0<T, Rest...>::value);
};

template <typename T>
struct index_of0<T> {
	static constexpr u64 value = 0;
	static_assert(sizeof(T) != 0, "Type not found in index_of Ts...");
};

template <typename T, typename... Ts>
struct index_of {
	static constexpr u64 value =
		index_of0<T, Ts...>::value == sizeof...(Ts) ? -1 : index_of0<T, Ts...>::value;
};

template <typename... Ts>
struct max_sizeof {
	static constexpr u64 value = max(sizeof(Ts)...);
};

template <typename... Ts>
struct max_alignof {
	static constexpr u64 value = max(alignof(Ts)...);
};

template <typename T>
[[nodiscard]] constexpr
typename remove_reference<T>::type&& move(typename remove_reference<T>::type& input) noexcept {
	return static_cast<typename remove_reference<T>::type&&>(input);
}

template<typename T>
constexpr T&& forward(typename remove_reference<T>::type& t) noexcept {
    return static_cast<T&&>(t);
}

template<typename T>
constexpr T&& forward(typename remove_reference<T>::type&& t) noexcept {
    // static_assert(!is_lvalue_reference_v<T>);
    return static_cast<T&&>(t);
}

namespace std {
	template<class _Ep2>
	struct initializer_list {
		using _Ep = remove_const<_Ep2>::type;

		const _Ep* __begin_;
		size_t    __size_;
		
		inline
		constexpr
		initializer_list(const _Ep* __b, size_t __s) noexcept
			: __begin_(__b),
			__size_(__s)
		{}

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
}

template <u64 N, typename T>
constexpr auto countof(const T (&arr)[N]) noexcept { return N; }

template <typename T, T v>
struct integral_const {
	static constexpr T value = v;
};

template <typename T>
struct is_enum {
	static constexpr bool value = integral_const<bool, __is_enum(T)>::value;
};
