#pragma once

#include <types.hh>
extern "C" {
	void memcpy(void* dst, void* src, u64 s);
	void memset(void* a, u8 c, u64 count);

	bool memcmp(void* a, void* b, u64 count); // returns false if equal
	bool memchk(void* a, u8 c, u64 count);
}
