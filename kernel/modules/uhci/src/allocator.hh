#pragma once

#include "uhci.hh"

#include <arch/amd64/paging.hh>

uhci_td_t* uhci_alloc_td();
uhci_qh_t* uhci_alloc_qh();
u32 uhci_alloc_page();
void uhci_free(void* p);

template <typename T>
u32 ulookup(T* ptr) {
	u64 asd = paging_lookup((u64)ptr);
	if constexpr (DBG) {
		if (asd >> 32)
			fatal("UHCI: out of 32 bit addresses! %p, %p", asd, ptr);
		assert(!(asd & 15));
	}
	return (u32)(u64)asd;
}
