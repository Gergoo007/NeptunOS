#pragma once

#include "types.hh"

struct Lock {
	volatile atomic u32 _lock = 0;
	inline void acquire() { wait(); _lock = 1; }
	inline void release() { _lock = 0; }
	inline void wait() const { while (_lock); }
};
