#include <util/mm.hh>
#include <cppcompat.hh>
#include <mm/pmm.hh>

MemoryMgr::MemoryMgr(u64 heap, u64 size): heap_base(heap) {
	capacity = PMM_PAGESIZE / sizeof(link_t);

	first = links = (link_t*)pmm_alloc(PMM_PAGESIZE);
	memset(links, 0, PMM_PAGESIZE);

	bm = new (bitmapStorage) Bitmap;

	bm->init((u64*)pmm_alloc(PMM_PAGESIZE), capacity);

	links[0] = link_t {
		.next = nullptr,
		.prev = nullptr,

		#ifdef VMM_DEBUG
		.file = __FILE__,
		.line = __LINE__,
		#endif

		.length = size,
		.free = true,
	};
	bm->set(0, true);

	freemem = size;
}

MemoryMgr::~MemoryMgr() { fatal("unimpl."); }

u32 MemoryMgr::count_allocs() {
	u32 ret = 0;
	link_t* l = first;
	while (l) {
		l = l->next;
		ret++;
	}

	return ret;
}

void MemoryMgr::delete_link(link_t* l) {
	// Külön változó hogy ne sírjon a clang
	u64 offset = (u64)l - (u64)links;
	u64 index = offset / sizeof(link_t);

	if (l->prev == nullptr)
		first = l->next;

	bm->set(index, false);
	l->next = nullptr;
	l->prev = nullptr;
	#ifdef VMM_DEBUG
	l->file = nullptr;
	l->line = 0;
	#endif
}

MemoryMgr::link_t* MemoryMgr::create_link() {
	u64 idx = bm->find_and_set();
	if (idx == -1ULL) {
		fatal(
			"Kifogyott a vmm bitmap! Hasznalt: %llu KiB (%llu MiB) Szabad: %llu KiB (%llu MiB) bm @ %p",
			bytes2kibs(pmm_usedmem),
			bytes2mibs(pmm_usedmem),
			bytes2kibs(pmm_freemem),
			bytes2mibs(pmm_freemem),
			bm->buffer
		);
	}

	#ifdef VMM_DEBUG
	links[idx].file = nullptr;
	links[idx].line = 0;
	#endif
	return &links[idx];
}

void MemoryMgr::merge(link_t* l) {
	// Összevonás az utána lévővel
	if (l->next) {
		link_t* old = l->next;
		if (l->free && l->next->free) {
			// A második link (l->next) eltávolítása
			l->length += l->next->length;
			if (l->next->next)
				l->next->next->prev = l;
			l->next = l->next->next;

			delete_link(old);
		}
	}

	// Összevonás az előtte lévővel
	if (l->prev) {
		if (l->free && l->prev->free) {
			// A második (l) link eltávolítása
			l->prev->length += l->length;
			if (l->next)
				l->next->prev = l->prev;
			l->prev->next = l->next;

			delete_link(l);
		}
	}
}

// Kisajátít egy free blokkot, és létrehoz egy újat ha maradt még az eredetiből
MemoryMgr::link_t& MemoryMgr::allocate_into_free(link_t* current, u64 size, u32 additional) {
	assert(current->free);
	current->free = false;

	assert(current->length >= size);
	assert(current->length - size >= additional);

	u64 rem = current->length - size - additional;
	
	// Ha maradt még a free linkből akkor kell utána egy újat csinálni
	// hiszen a current az éppen lefoglalt link
	if (rem) {
		link_t* newlink = create_link();

		// be kell állítani: a currentet, a new-t és a következőt
		current->length = size;

		newlink->next = current->next;
		newlink->prev = current;
		newlink->length = rem;
		newlink->free = true;

		if (current->next)
			current->next->prev = newlink;

		current->next = newlink;

		return *newlink;
	}
	return *current;
}

void* MemoryMgr::alloc_nomutex(u64 size, const char* file, u32 line) {
	if (!size) return nullptr;
	size = align(size, VMM_MIN_ALLOC);

	// Setup redzones before and after the allocation
	#ifdef VMM_DEBUG
	size += VMM_REDZONE_SIZE * 2;
	#endif

	link_t* l = first;
	u64 address = heap_base;
	while (l) {
		if (l->free && l->length >= size)
			break;

		address += l->length;
		l = l->next;
	}

	// current átállítása a used linkké, majd egy új free link beillesztése utána
	allocate_into_free(l, size, 0);
	#ifdef VMM_DEBUG
	l->file = file;
	l->line = line;
	#endif

	usedmem += size;
	freemem -= size;

	// Fill redzones with magic value
	#ifdef VMM_DEBUG
	memset((void*)address, VMM_REDZONE_MAGIC, VMM_REDZONE_SIZE);
	memset((void*)(address + size - VMM_REDZONE_SIZE), VMM_REDZONE_MAGIC, VMM_REDZONE_SIZE);
	address += VMM_REDZONE_SIZE;
	#endif

	return (void*)address;
}

void* MemoryMgr::alloc(u64 size, const char* file, u32 line) {
	LockguardSimple g(m);
	auto p = alloc_nomutex(size, file, line);
	// if ((u64)p == 0xffff9000003e8aa0) pause();
	return p;
}

void* MemoryMgr::alloc_aligned(u64 size, u32 align, const char* file, u32 line) {
	LockguardSimple g(m);
	size = align(size, VMM_MIN_ALLOC);
	align = align(align, VMM_MIN_ALLOC);

	#ifdef VMM_DEBUG
	constexpr u64 rzcorr = VMM_REDZONE_SIZE;
	size += VMM_REDZONE_SIZE * 2;
	#else
	constexpr u64 rzcorr = 0;
	#endif

	u64 address = heap_base;
	link_t* l = first;
	while (l) {
		if (!l->free) goto cont;
		if (l->length < size) goto cont;

		if (isaligned(address + rzcorr, align)) {
			allocate_into_free(l, size, 0);
			#ifdef VMM_DEBUG
			l->file = file;
			l->line = line;
			#endif

			break;
		} else if (l->length > size) {
			u64 alignfix = align(address + rzcorr, align) - (address + rzcorr);
			assert(alignfix);

			if (l->length < size + alignfix) goto cont;

			// A VMM lánclistának nagyjából így kell átváltoznia:
			// ... l ...  ->  ... alignfixl l remainingl ...

			link_t* alignfixl = create_link();
			alignfixl->free = true;
			alignfixl->length = alignfix;
			l->length -= alignfix;
			assert(isaligned(alignfixl->length, VMM_MIN_ALLOC));

			#ifdef VMM_DEBUG
			alignfixl->file = file;
			alignfixl->line = line;
			#endif

			if (l->prev)
				l->prev->next = alignfixl;
			alignfixl->prev = l->prev;
			alignfixl->next = l;
			l->prev = alignfixl;

			if (l == first)
				first = alignfixl;

			i64 remaining = l->length - size;
			sprintk("%llx vs %llx + %llx\r\n", l->length, size, alignfix);
			sprintk("rem %llx\r\n", remaining);
			assert(remaining >= 0ll);
			l->length -= remaining;
			l->free = false;
			#ifdef VMM_DEBUG
			l->file = file;
			l->line = line;
			#endif
			if (remaining) {
				// remainingl beillesztése, mivel ide kell
				link_t* remainingl = create_link();
				#ifdef VMM_DEBUG
				remainingl->file = __FILE__;
				remainingl->line = __LINE__;
				#endif
				remainingl->free = true;
				remainingl->length = remaining;
				assert(isaligned(remainingl->length, 16));

				if (l->next)
					l->next->prev = remainingl;
				remainingl->next = l->next;
				l->next = remainingl;
				remainingl->prev = l;
			}

			address += alignfix;

			break;
		}
cont:
		address += l->length;
		l = l->next;
	}

	usedmem += size;
	freemem -= size;

	#ifdef VMM_DEBUG
	memset((void*)address, VMM_REDZONE_MAGIC, VMM_REDZONE_SIZE);
	memset((void*)(address + size - VMM_REDZONE_SIZE), VMM_REDZONE_MAGIC, VMM_REDZONE_SIZE);
	address += VMM_REDZONE_SIZE;
	#endif

	return (void*)address;
}

// If the sector can be increased in size without alloc+memcpy,
// do that and return true, otherwise dont do anythign and return false
bool MemoryMgr::try_realloc(void* ptr, u64 newsize, const char* file, u32 line) {
	LockguardSimple g(m);
	if (!ptr)
		return alloc_nomutex(newsize, file, line);

	assert(((u64)ptr & 15) == 0);

	link_t* i = first;
	u64 addr = heap_base;
	u64 oldsize = 0;
	newsize = align(newsize, VMM_MIN_ALLOC);

	#ifdef VMM_DEBUG
	ptr = (void*) ((u64)ptr - VMM_REDZONE_SIZE);
	newsize += VMM_REDZONE_SIZE * 2;
	#endif

	while (i) {
		if (addr == (u64)ptr) {
			oldsize = i->length;
			if (newsize <= oldsize)
				goto allocd;

			if (i->next && i->next->free) {
				// TODO: Ha kisebb lesz a maradék (i->next) mint MIN_ALLOC + REDZONE_SIZE, akkor
				// használhatatlan lesz amúgy is, szóval jobban járunk ha beleolvasztjuk a jelenlegibe

				if (i->next->length == newsize - i->length) {
					i->next->length -= newsize - i->length;
					i->length += newsize - i->length;
					usedmem += (newsize - oldsize);
					freemem -= (newsize - oldsize);

					// i->next törlése
					link_t* old = i->next;
					i->next = old->next;
					if (old->next)
						old->next->prev = i;
					delete_link(old);

					goto allocd;
				} else if (i->next->length > newsize - i->length) {
					i->next->length -= newsize - i->length;
					i->length += newsize - i->length;
					usedmem += (newsize - oldsize);
					freemem -= (newsize - oldsize);
				
					goto allocd;
				} else {
					return false;
				}
			} else {
				return false;
			}
		}

		addr += i->length;
		i = i->next;
	}

	fatal("Elerhetetlen kod! ptr invalid? %p", ptr);

allocd:
	#ifdef VMM_DEBUG
	i->file = file;
	i->line = line;

	memset((void*)((u64)ptr + i->length - VMM_REDZONE_SIZE), VMM_REDZONE_MAGIC, VMM_REDZONE_SIZE);
	#endif
	return true;
}

u64 MemoryMgr::get_size(void* p) {
	link_t* i = first;
	u64 addr = heap_base;

	#ifdef VMM_DEBUG
	p = (void*)((u64)p - VMM_REDZONE_SIZE);
	#endif
	
	while (i) {
		if (addr == (u64)p) {
			u64 size = i->length;
			#ifdef VMM_DEBUG
			size -= VMM_REDZONE_SIZE * 2;
			#endif
			return size;
		}
		addr += i->length;
		i = i->next;
	}

	fatal("Unreachable code in get_size!");
}

void* MemoryMgr::realloc(void* ptr, u64 newsize, const char* file, u32 line) {
	if (try_realloc(ptr, newsize, file, line)) {
		return ptr;
	} else {
		void* newmem = alloc(newsize, file, line);
		u64 oldsize = get_size(ptr);
		memcpy(newmem, ptr, oldsize);
		free(ptr, __FILE__, __PRETTY_FUNCTION__);
		return newmem;
	}
}


u64 MemoryMgr::dump() {
	LockguardSimple g(m);
	printk("===============================\n");
	link_t* i = first;
	u64 addr = heap_base;
	while (i) {
		#ifdef VMM_DEBUG
		printk("[%p] %s: %08llx byte [%s:%d]\n", (void*)(addr + VMM_REDZONE_SIZE), i->free ? "FREE" : "USED", i->length, i->file, i->line);
		#else
		printk("[%p] %s: %08llx byte\n", (void*)addr, i->free ? "FREE" : "USED", i->length);
		#endif
		addr += i->length;
		i = i->next;
	}
	printk("===============================\n");
	return addr - heap_base;
}

void MemoryMgr::info(void* p) {
	LockguardSimple g(m);
	link_t* i = first;
	u64 addr = heap_base;

	while (i) {
		if (addr == (u64)p) {
			report("itt az info %p:", p);
			report("szabad: %d; meret: %lld (%llx)", i->free, i->length, i->length);
			report("next: %p prev: %p", i->next, i->prev);
			return;
		}

		addr += i->length;
		i = i->next;
	}

	fatal("Nincs allokacio ilyen cimen: %p", p);
}

void MemoryMgr::free_nomutex(void* p, const char* file, const char* function) {
	if (!p) return;

	check(p);

	u64 addr = heap_base;
	u64 linksize = -1;
	link_t* i = first;

	#ifdef VMM_DEBUG
	p = (void*)((u64)p - VMM_REDZONE_SIZE);
	#endif

	while (i) {
		if (addr == (u64)p) {
			linksize = i->length;
			break;
		}

		addr += i->length;
		i = i->next;
	}

	#ifdef VMM_DEBUG
	if (linksize == -1ULL)
		fatal("turi ipő ip");
	if (!i || addr != (u64)p)
		fatal("Ervenytelen free! [%p]\nFILE %s\nFUNC %s", p, file, function);

	i->file = nullptr;
	i->line = 0;
	#else
	if (!i || addr != (u64)p)
		fatal("Ervenytelen free! [%p]", p);
	#endif

	usedmem -= linksize;
	freemem += linksize;

	i->free = true;

	merge(i);
}

void MemoryMgr::free(void* p, const char* file, const char* function) {
	LockguardSimple g(m);
	free_nomutex(p, file, function);
}

void MemoryMgr::print_files(void* around) {
	LockguardSimple g(m);
	#ifdef VMM_DEBUG
		link_t* l = first;
		u64 addr = heap_base;
		while (l) {
			if (addr == (u64)around)
				break;
			addr += l->length;
			l = l->next;
		}

		if (l->prev)
			report("Prev   alloc [%p, %d]: %s:L%d", (void*)(addr - l->prev->length), (u32)l->length, l->prev->file, l->prev->line);
		report("Callee alloc [%p, %d]: %s:L%d", (void*)(addr), (u32)l->length, l->file, l->line);
		if (l->next)
			report("Next   alloc [%p, %d]: %s:L%d", (void*)(addr + l->length), (u32)l->length, l->next->file, l->next->line);
	#else
		error("print_files needs to be enabled by defining VMM_DEBUG!");
	#endif
}

void MemoryMgr::check(void* p, bool checkbefore, bool checkafter) {
	#ifdef VMM_DEBUG
	p = (void*)((u64)p - VMM_REDZONE_SIZE);

	u64 addr = heap_base;
	u64 linksize = -1;
	link_t* i = first,* j = first;

	while (i) {
		if (addr == (u64)p) {
			linksize = i->length;
			break;
		}

		j = i;
		addr += i->length;
		i = i->next;
	}

	if (linksize == -1ULL) {
		sprintk("No such alloc: %p\n\r", p);
		fatal("No such alloc: %p", p);
	}

	if (i->free) {
		sprintk("Tried to check a free sector?? %p", p);
		fatal("Tried to check a free sector?? %p", p);
	}

	if (i->length < VMM_MIN_ALLOC + VMM_REDZONE_SIZE * 2) {
		con_clear();
		dump();
		fatal("%p is only 0x%llx bytes?? [%s:%d]", p, i->length, i->file, i->line);
	}

	if (memchk(p, VMM_REDZONE_MAGIC, VMM_REDZONE_SIZE)) {
		sprintk("Redzone corruption before allocation (%p)!\n\rAllocated @ %s:%d\n\r", (void*)((u64)p+VMM_REDZONE_SIZE), i->file, i->line);
		error("Redzone corruption before allocation (%p)!\nAllocated @ %s:%d", (void*)((u64)p+VMM_REDZONE_SIZE), i->file, i->line);
		void* p2 = (void*)((u64)p - j->length);
		error("Relevant neighbor: %s %p, 0x%llx bytes @ %s:%d", j->free ? "FREE" : "USED", p2, j->length, j->file, j->line);
		assert(j->length == get_size((void*)((u64)p2 + 64)) + VMM_REDZONE_SIZE * 2);
		bool firstredzone = memchk(p2, VMM_REDZONE_MAGIC, VMM_REDZONE_SIZE);
		bool secondredzone = memchk((void*)((u64)p2 + j->length - VMM_REDZONE_SIZE), VMM_REDZONE_MAGIC, VMM_REDZONE_SIZE);
		if (firstredzone) error("Neighbor's first redzone is corrupted as well!");
		if (secondredzone) error("Neighbor's second redzone is corrupted as well!");
		if (!(firstredzone || secondredzone)) error("Neighbor's redzones are intact.");
		error("=================================");
		if (checkbefore) check((void*)((u64)p2 + VMM_REDZONE_SIZE), true, false);
		pause();
	}

	p = (void*)((u64)p + linksize - VMM_REDZONE_SIZE);
	if (memchk(p, VMM_REDZONE_MAGIC, VMM_REDZONE_SIZE)) {
		sprintk("Redzone corruption after allocation (%p)!\n\rAllocated @ %s:%d\n\r", (void*)((u64)p+VMM_REDZONE_SIZE), i->file, i->line);
		error("Redzone corruption after allocation (%p)!\nAllocated @ %s:%d", (void*)((u64)p+VMM_REDZONE_SIZE), i->file, i->line);
		void* p2 = (void*)((u64)p + i->length);
		j = i->next;
		error("Relevant neighbor: %s %p, 0x%llx bytes @ %s:%d", j->free ? "FREE" : "USED", p2, j->length, j->file, j->line);
		bool firstredzone = memchk(p2, VMM_REDZONE_MAGIC, VMM_REDZONE_SIZE);
		bool secondredzone = memchk((void*)((u64)p2 + j->length - VMM_REDZONE_SIZE), VMM_REDZONE_MAGIC, VMM_REDZONE_SIZE);
		if (firstredzone) error("Neighbor's first redzone is corrupted as well!");
		if (secondredzone) error("Neighbor's second redzone is corrupted as well!");
		if (!(firstredzone || secondredzone)) error("Neighbor's redzones are intact.");
		error("=================================");
		if (checkafter) check((void*)((u64)p2 + VMM_REDZONE_SIZE), false, true);
		pause();
	}
	#endif
}

void MemoryMgr::check_all() {
	link_t* l = first;
	u64 a = heap_base;
	while (l) {
		if (!l->free) {
			check((void*)(a + VMM_REDZONE_SIZE));
		}

		a += l->length;
		l = l->next;
	}
}