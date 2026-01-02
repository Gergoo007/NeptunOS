#pragma once

#include <types.hh>

pstruct stackframe_t {
	stackframe_t* rbp;
	u64 rip;
};

void stacktrace(u64 rsp = -1, void (*_printk)(const char* fmt, ...) = printk);
