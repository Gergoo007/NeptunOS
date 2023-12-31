#pragma once

#include <lib/int.h>

inline u64 abs64(i64 num) {
	return (num < 0) ? (0xffffffffffffffff - num + 1) : (num);
}
