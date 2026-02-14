#include <mm/pmm4g.hh>
#include <mm/pmm.hh>
#include <util/mem.hh>
#include <cppcompat.hh>

u8 _g_pmm4g[sizeof(memorymgr)];
memorymgr& g_pmm4g = *(memorymgr*)_g_pmm4g;

void pmm4g_init(u64 heap_base, u64 size) {
	sprintk("pmm4g @ %p of %lld kibs\n\r", (void*)heap_base, bytes2kibs(size));
	new (_g_pmm4g) memorymgr(VIRTUAL(heap_base), size);
}
