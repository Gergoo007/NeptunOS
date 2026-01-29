#pragma once

#include <types.hh>
#include <util/helpers.hh>

template <typename... Ts>
struct variant {
	alignas(max_alignof<Ts...>::value)
	u8 storage[max_sizeof<Ts...>::value];

	static constexpr u64 inv = (u8)-1;
	u8 active = inv;

	variant() {  }

	template <typename T>
	variant(bool asd, T&& elem) {
		active = index_of<T, Ts...>::value;
		assert(active != -1ull);
		new (&storage) T(move<T>(elem));
	}

	template <typename T>
	variant(bool asd, const T& elem) {
		active = index_of<T, Ts...>::value;
		new (&storage) T(elem);
	}

	template <typename T, typename... Args>
	void emplace_back(Args&&... args) {
		if (active != inv) fatal("variant already has an active element!");
		active = index_of<T, Ts...>::value;
		new (&storage) T(forward<Args>(args)...);
	}

	template <typename T>
	T& get() {
		if (active == inv) fatal("variant has no active element!");
		u64 idx = index_of<T, Ts...>::value;
		
		if (idx != active) fatal("Type mismatch! active: %d; requested: %lld", active, idx);
		return *(T*)storage;
	}

	void destroy() {
		if (active == inv) return;
		u64 i = 0;
		((active == i++ ? ( ((Ts*)storage)->~Ts() ) : void(0)), ...);
		active = inv;
	}

	~variant() {
		destroy();
	}
};
