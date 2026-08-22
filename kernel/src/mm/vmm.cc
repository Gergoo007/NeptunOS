#include <mm/vmm.hh>
#include <mm/pmm.hh>
#include <cppcompat.hh>
#include <util/async.hh>
#include <gfx/console.hh>

u8 _g_vmm[sizeof(MemoryMgr)];
MemoryMgr& g_vmm = *(MemoryMgr*)_g_vmm;

// 16 terabyte-al a HHDM kezdete után
constexpr u64 VMM_HEAP_BASE = 0xffff900000000000;

void vmm_init() { new (_g_vmm) MemoryMgr(VMM_HEAP_BASE, pmm_freemem); }

// watermark allocator
u64 wm_cursor = 0;
u64 wm_free = 0;
MutexSimple wm_mutex;

void* wm_alloc(u64 size, u64 align) {
	LockguardSimple turi(wm_mutex);

	size = align(size, 16);
	align = max(size, align);

	u64 alignfix = align(wm_cursor, align) - wm_cursor;
	wm_cursor += alignfix;

	void* p = (void*)wm_cursor;
	wm_cursor += size;

	if (wm_free < size + alignfix)
		fatal("Watermark allocator out of memory!");
	wm_free -= size + alignfix;

	return p;
}
