#pragma once

#include <mm/pmm.hh>
#include <util/bitmap.hh>

template <typename T>
struct List {
	struct Link {
		Link* next = nullptr;
		Link* prev = nullptr;
		T data;
	};

	Link* links;
	u64 capacity;
	Bitmap bm;

	void init_pmm();
	void insert();
};
