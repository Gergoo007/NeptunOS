#pragma once

#include <types.hh>

pstruct Stackframe {
	Stackframe* rbp;
	u64 rip;
};

void stacktrace();
