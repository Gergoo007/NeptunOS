#include <mm/pmm4g.hh>
#include <mm/pmm.hh>
#include <util/mem.hh>
#include <cppcompat.hh>

#define PMM4G_DEBUG 1

struct link_t {
	link_t* next;
	link_t* prev;

	#ifdef PMM4G_DEBUG
	const char* file;
	u32 line;
	#endif

	u64 length;
	bool free;
};

link_t* pmm4g_links;
link_t* pmm4g_first;
u8 pmm4g_bitmapStorage[sizeof(bitmap_t)];
bitmap_t* pmm4g_bm;
u64 pmm4g_capacity;

u64 pmm4g_usedmem = 0;
u64 pmm4g_freemem;

u64 pmm4g_heap_base;
u64 pmm4g_heap_size;

constexpr u32 MIN_ALLOC = 64;

void pmm4g_init(u64 heap_base, u64 size) {
	pmm4g_first = pmm4g_links = (link_t*)pmm_alloc();

	memset(pmm4g_links, 0, pmm_pagesize);
	pmm4g_capacity = pmm_pagesize / sizeof(link_t);

	pmm4g_bm = new (pmm4g_bitmapStorage) bitmap_t;

	pmm4g_bm->init((u64*)pmm_alloc(), pmm4g_capacity);

	pmm4g_links[0] = link_t {
		.next = nullptr,
		.prev = nullptr,

		#ifdef PMM4G_DEBUG
		.file = "",
		.line = 0,
		#endif

		.length = size,
		.free = true,
	};
	pmm4g_bm->set(0, true);

	pmm4g_freemem = size;

	pmm4g_heap_base = VIRTUAL(heap_base);
	pmm4g_heap_size = size;
}

// TODO: "jelző" változó, ami az utolsó free blokkra mutat mindig,
// ezáltal nem kell minden alkalommal végigiterálni a linkelt listát
// Ha eléri ez a jelző a heap végét, be kell állítani -1-re, mellyel
// jelzi hogy már csak szabadított blokkok vannak, szétszórva

u32 pmm4g_count_allocs() {
	u32 ret = 0;
	link_t* l = pmm4g_first;
	while (l) {
		l = l->next;
		ret++;
	}

	return ret;
}

static void delete_link(link_t* l) {
	// Külön változó hogy ne sírjon a clang
	u64 offset = (u64)l - (u64)pmm4g_links;
	u64 index = offset / sizeof(link_t);

	if (l->prev == nullptr)
		pmm4g_first = l->next;

	pmm4g_bm->set(index, false);
	l->next = (link_t*)0x6966969696969669;
	l->prev = (link_t*)0x6966969696969669;
}

static link_t* create_link() {
	u64 idx = pmm4g_bm->find_and_set();
	if (idx == -1ULL) {
		fatal(
			"Kifogyott a pmm4g bitmap! Hasznalt: %llu KiB (%llu MiB) Szabad: %llu KiB (%llu MiB)",
			bytes2kibs(pmm_usedmem),
			bytes2mibs(pmm_usedmem),
			bytes2kibs(pmm_freemem),
			bytes2mibs(pmm_freemem)
		);
	}
	return &pmm4g_links[idx];
}

void pmm4g_merge(link_t* l) {
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
static link_t& allocate_into_free(link_t* current, u64 size, u32 additional) {
	current->free = false;
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

void* pmm4g_alloc(u64 size, const char* file, u32 line) {
	if (!size) return nullptr;
	size = align(size, MIN_ALLOC);

	if (size > pmm4g_freemem)
		fatal("TODO: Out of 4g memory!");

	link_t* current = pmm4g_first;
	u64 address = pmm4g_heap_base;
	while (!current->free || current->length <= size) {
		address += current->length;
		current = current->next;
		if (!current) {
			pmm4g_dump();
			fatal(
				"Elfogyott a memoria!\npmm used vs free %llu MiB %llu MiB\nvmm used vs free %llu MiB %llu MiB",
				bytes2mibs(pmm_usedmem), bytes2mibs(pmm_freemem),
				bytes2mibs(pmm4g_usedmem), bytes2mibs(pmm4g_freemem)
			);
		}
	}

	// current átállítása a used linkké, majd egy új free link beillesztése utána
	auto& l = allocate_into_free(current, size, 0);
	(void)l;
	#ifdef PMM4G_DEBUG
		l.file = file;
		l.line = line;
	#endif

	pmm4g_usedmem += size;
	pmm4g_freemem -= size;
	return (void*)address;
}

void* pmm4g_alloc_aligned(u64 size, u32 align, const char* file, u32 line) {
	size = align(size, MIN_ALLOC);
	align = align(align, MIN_ALLOC);

	link_t* l = pmm4g_first;
	u64 address = pmm4g_heap_base;
	while (l) {
		u64 alignfix = 0;
		// minimum ekkorának kell lennie a free blokknak az igazítás miatt
		if (address & (align-1))
			alignfix = align - (address & (align-1));

		u64 minsize = size + alignfix;
		if (l->free && l->length >= minsize) {
			// Itt (lehet hogy) kell egy free blokk, aztán
			// kell egy used, aztán (lehet hogy) kell még egy free
			if (l->length > minsize && alignfix) {
				// Kell egy free blokk először
				link_t* freelink = create_link();
				if (l->prev)
					l->prev->next = freelink;
				freelink->prev = l->prev;
				l->prev = freelink;
				freelink->next = l;

				freelink->free = true;
				freelink->length = alignfix;
			}
			address += alignfix;

			// Innentől mehet a normális alloc procedúra
			// az alignfix-et is le kell vonni, külön a size-tól
			auto link = allocate_into_free(l, size, alignfix);
			(void)link;
			#ifdef PMM4G_DEBUG
			link.file = file;
			link.line = line;
			#endif

			break;
		}

		address += l->length;
		l = l->next;
	}

	pmm4g_usedmem += size;
	pmm4g_freemem -= size;
	return (void*)address;
}

void* pmm4g_realloc(void* ptr, u64 newsize) {
	if (!ptr)
		return kmalloc4g(newsize);

	assert(((u64)ptr & (MIN_ALLOC-1)) == 0);

	link_t* i = pmm4g_first;
	u64 addr = pmm4g_heap_base;
	u64 oldsize = 0;
	newsize = align(newsize, MIN_ALLOC);

	while (i) {
		if (addr == (u64)ptr) {
			oldsize = i->length;
			if (newsize <= oldsize)
				return ptr;

			if (i->next) {
				if (i->next->length == newsize - i->length) {
					i->next->length -= newsize - i->length;
					i->length += newsize - i->length;
					pmm4g_usedmem += (newsize - oldsize);
					pmm4g_freemem -= (newsize - oldsize);

					// i->next törlése
					link_t* old = i->next;
					i->next = old->next;
					old->next->prev = i;
					delete_link(old);

					return ptr;
				} else if (i->next->length > newsize - i->length) {
					i->next->length -= newsize - i->length;
					i->length += newsize - i->length;
					pmm4g_usedmem += (newsize - oldsize);
					pmm4g_freemem -= (newsize - oldsize);
					return ptr;
				} else {
					goto whatever;
				}
			} else {
				goto whatever;
			}
		}

		addr += i->length;
		i = i->next;
	}

	fatal("Elerhetetlen kod! ptr invalid? %p", ptr);

whatever:
	void* newloc = kmalloc4g(newsize);
	memcpy(newloc, ptr, oldsize);
	kfree4g(ptr);
	return newloc;
}

u64 pmm4g_dump() {
	printk("===============================\n");
	link_t* i = pmm4g_first;
	u64 addr = pmm4g_heap_base;
	while (i) {
		printk("[%p] %s: %08llx byte\n", (void*)addr, i->free ? "FREE" : "USED", i->length);
		addr += i->length;
		i = i->next;
	}
	printk("===============================\n");
	return addr - pmm4g_heap_base;
}

void pmm4g_info(void* p) {
	link_t* i = pmm4g_first;
	u64 addr = pmm4g_heap_base;

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

void pmm4g_free(void* p, const char* file, const char* function) {
	if (!p) return;

	u64 addr = pmm4g_heap_base;
	u64 linksize = -1;
	link_t* i = pmm4g_first;

	while (i) {
		if (addr == (u64)p) {
			linksize = i->length;
			break;
		}

		addr += i->length;
		i = i->next;
	}

	#ifdef PMM4G_DEBUG
	if (linksize == -1ULL)
		fatal("turi ipő ip");
	if (!i || addr != (u64)p)
		fatal("Ervenytelen free! [%p]\nFILE %s\nFUNC %s", p, file, function);
	#else
	if (!i || addr != (u64)p)
		fatal("Ervenytelen free! [%p]", p);
	#endif

	if (i == (link_t*)0x6966969696969669)
		fatal("nem jo.. ");

	pmm4g_usedmem -= linksize;
	pmm4g_freemem += linksize;

	i->free = true;

	pmm4g_merge(i);
}

void pmm4g_print_files(void* around) {
	#ifdef PMM4G_DEBUG
		link_t* l = pmm4g_first;
		u64 addr = pmm4g_heap_base;
		while (l) {
			if (addr == (u64)around)
				break;
			addr += l->length;
			l = l->next;
		}

		if (l->prev)
			report("Prev   alloc [%p, %lld]: %s:L%d", (void*)(addr - l->prev->length), l->length, l->prev->file, l->prev->line);
		report("Callee alloc [%p, %lld]: %s:L%d", (void*)(addr), l->length, l->file, l->line);
		if (l->next)
			report("Next   alloc [%p, %lld]: %s:L%d", (void*)(addr + l->length), l->length, l->next->file, l->next->line);
	#else
		error("pmm4g_print_files needs to be enabled by defining PMM4G_DEBUG!")
	#endif
}
