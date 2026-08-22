#pragma once

#include <util/storage.hh>

struct page_table_t;
struct program {
	void (*entry)(int argc, char** argv);
	page_table_t* cr3;
	u64 stack;

	program(Span<u8> exefile);
	~program();
	
	void launch();
};
