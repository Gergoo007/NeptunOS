#pragma once

#include <arch/arch.hh>

struct mutex {
	int lockvar = 0;

    void lock() {
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

	lockguard(mutex& m): m(m) { m.lock(); }
	lockguard(const lockguard&) = delete("ha");
	lockguard(lockguard&&) = delete("asd");
	lockguard& operator=(const lockguard&) = delete("88");
	~lockguard() { m.unlock(); }
};
