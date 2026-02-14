#pragma once

#include <util/bitmap.hh>
#include <util/async.hh>
#include <mm/pmm.hh>
#include <cppcompat.hh>

#define VMM_DEBUG 1

constexpr u64 VMM_MIN_ALLOC = 16;

struct memorymgr {
	struct link_t {
		link_t* next;
		link_t* prev;

		#ifdef VMM_DEBUG
		const char* file;
		u32 line;
		#endif

		u64 length;
		bool free;
	};

	link_t* links;
	link_t* first;
	u8 bitmapStorage[sizeof(bitmap_t)];
	bitmap_t* bm;
	u64 capacity;

	u64 usedmem = 0;
	u64 freemem;

	const u64 heap_base;

	mutex m;

	memorymgr(u64 heap, u64 size);
	~memorymgr();

	u32 count_allocs();
	void* alloc(u64 size, const char* file, u32 line);
	void* alloc_aligned(u64 size, u32 align, const char* file, u32 line);
	void* realloc(void* ptr, u64 newsize, const char* file, u32 line);
	void free(void* p, const char* file, const char* function);
	void info(void* p);
	u64 dump();
	void print_files(void* around);
	void check(void* p, bool checkbefore = true, bool checkafter = true);
	void check_all();
	bool try_realloc(void* ptr, u64 newsize, const char* file = "TRY_REALLOC_NOT_SPECIFIED", u32 line = 0);
	u64 get_size(void* p);

private:
	void delete_link(link_t* l);
	link_t* create_link();
	void merge(link_t* l);
	link_t& allocate_into_free(link_t* current, u64 size, u32 additional);
	void* alloc_nomutex(u64 size, const char* file, u32 line);
	void free_nomutex(void* p, const char* file, const char* function);
};
