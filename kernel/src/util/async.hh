#pragma once

#include <types.hh>
#include <util/helpers.hh>

struct mutex {
	int lockvar = 0;

    void lock() {
		// cmpxchg is basically: if (lockvar == expected) lockvar = 1;
        int expected = 0;
        while (
			!__atomic_compare_exchange_n(
				&lockvar, &expected, 1, false,
				__ATOMIC_ACQUIRE, __ATOMIC_RELAXED
			)
		) {
            expected = 0;  // reset after failed CAS
            asm volatile ("pause" ::: "memory");
        }
    }

    void unlock() { __atomic_store_n(&lockvar, 0, __ATOMIC_RELEASE); }
};

struct lockguard {
	mutex& m;

	lockguard(mutex& _m): m(_m) { m.lock(); }
	lockguard(const lockguard&) = delete("ha");
	lockguard(lockguard&&) = delete("asd");
	lockguard& operator=(const lockguard&) = delete("88");
	~lockguard() { m.unlock(); }
};

template <typename T>
struct atomic {
	// using T = int;
	T data;

	constexpr atomic(T v): data(v) {  }
	constexpr atomic(): data() {  }

	constexpr bool is_lock_free() const { return __atomic_is_lock_free(sizeof(T), &data); }

	constexpr void store(T v, int order = __ATOMIC_SEQ_CST) {
		if (__builtin_is_constant_evaluated())
			data = v;
		else
			__atomic_store(&data, &v, order);
	}

	constexpr T load(int order = __ATOMIC_SEQ_CST) const {
		if (__builtin_is_constant_evaluated())
			return data;
		T ret;
		__atomic_load(&data, &ret, order);
		return ret;
	}

	atomic& operator=(const atomic& other) {
		store(other.load());
		return *this;
	}

	atomic& operator=(const T& other) {
		store(other);
		return *this;
	}

	atomic operator+(const T& other) const {
		return atomic(load() + other);
	}

	atomic operator-(const T& other) const {
		return atomic(load() - other);
	}

	atomic& operator+=(const T& other) {
		__atomic_fetch_add(&data, other, __ATOMIC_SEQ_CST);
		return *this;
	}

	atomic& operator-=(const T& other) {
		__atomic_fetch_sub(&data, other, __ATOMIC_SEQ_CST);
		return *this;
	}

	// Post-inc
	constexpr atomic operator++(int) {
		if (__builtin_is_constant_evaluated())
			return data++;
		else
			return atomic(__atomic_fetch_add(&data, 1, __ATOMIC_SEQ_CST));
	}

	// Post-dec
	constexpr atomic operator--(int) {
		if (__builtin_is_constant_evaluated())
			return data--;
		else
			return atomic(__atomic_fetch_sub(&data, 1, __ATOMIC_SEQ_CST));
	}

	constexpr atomic& operator++() {
		if (__builtin_is_constant_evaluated())
			data++;
		else
			__atomic_fetch_add(&data, 1, __ATOMIC_SEQ_CST);
		return *this;
	}

	constexpr atomic& operator--() {
		if (__builtin_is_constant_evaluated())
			data--;
		else
			__atomic_fetch_sub(&data, 1, __ATOMIC_SEQ_CST);
		return *this;
	}

	// Ha T esetleg egy pointer lenne
	typename remove_ptr<T>::type& operator*() {
		return *load();
	}

	T operator->() {
		return load();
	}

	operator T() const {
		return load();
	}
};
