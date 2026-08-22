#pragma once

#include <types.hh>
#include <util/helpers.hh>

struct MutexSimple {
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
            expected = 0;
            asm volatile ("pause" ::: "memory");
        }
    }

    void unlock() { __atomic_store_n(&lockvar, 0, __ATOMIC_RELEASE); }
};

template <typename T>
struct lockguard {
	lockguard(T& d, MutexSimple& m): data(d), mut(m) {
		mut.lock();
	}

	T& operator->() {
		return data;
	}

	~lockguard() {
		mut.unlock();
	}

private:
	T& data;
	MutexSimple& mut;
};

template <typename T>
struct mutex {
	T data;
	MutexSimple m;

	template <typename... Args>
	mutex(Args&&... args): data(forward<Args>(args)...) {}

	lockguard<T> lock() {
		return lockguard(data, m);
	}

	void unlock() {
		m.unlock();
	}

	const T& get() const {
		return data;
	}
};

struct LockguardSimple {
	MutexSimple& m;

	LockguardSimple(MutexSimple& _m): m(_m) { m.lock(); }
	LockguardSimple(const LockguardSimple&) = delete("ha");
	LockguardSimple(LockguardSimple&&) = delete("asd");
	LockguardSimple& operator=(const LockguardSimple&) = delete("88");
	~LockguardSimple() { m.unlock(); }
};

template <typename T>
struct atomic {
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
