#include <mm/vmm.hh>
#include <cppcompat.hh>

namespace vmm {
	Link* links;
	Link* first;
	u8 bitmapStorage[sizeof(Bitmap)];
	Bitmap* bm;
	u64 capacity;

	void init() {
		links = first = (Link*)pmm::alloc();
		memset(links, 0, pmm::pagesize);
		capacity = pmm::pagesize / sizeof(Link);

		bm = new (bitmapStorage) Bitmap;

		bm->init((u64*)pmm::alloc(), capacity);

		links[0] = Link {
			.next = nullptr,
			.prev = nullptr,
			.length = pmm::free,
			.free = true,
		};
		bm->set(0, true);

		#ifdef TRACE_ALLOCS
		report("vmm init\n");
		#endif
	}

	void delete_link(Link* l) {
		// Külön változó hogy ne sírjon a clang
		u64 offset = (u64)l - (u64)links;
		u64 index = offset / sizeof(Link);

		bm->set(index, false);
		l->next = (Link*)0x6966969696969669;
		l->prev = (Link*)0x6966969696969669;
	}

	Link* create_link() {
		return &links[bm->find_and_set()];
	}

	void merge(Link* l) {
		// Összevonás az utána lévővel
		if (l->next) {
			Link* old = l->next;
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
	void allocate_into_free(Link* current, u64 size, u32 additional) {
		current->free = false;
		u64 rem = current->length - size - additional;
		
		// Ha maradt még a free linkből akkor kell utána egy újat csinálni
		// hiszen a current az éppen lefoglalt link
		if (rem) {
			Link* newlink = create_link();

			// be kell állítani: a currentet, a new-t és a következőt
			current->length = size;

			newlink->next = current->next;
			newlink->prev = current;
			newlink->length = rem;
			newlink->free = true;

			if (current->next)
				current->next->prev = newlink;

			current->next = newlink;
		}
	}

	void* alloc(u64 size) {
		if (!size) return nullptr;
		size = align(size, 16);

		Link* current = first;
		u64 address = heap_base;
		while (!current->free || current->length < size) {
			address += current->length;

			current = current->next;
			if (!current)
				fatal("Elfogyott a memoria!\n");
		}

		// current átállítása a used linkké, majd egy új free link beillesztése utána
		allocate_into_free(current, size, 0);

		#ifdef TRACE_ALLOCS
		report("alloc %llx\n", size);
		#endif

		return (void*)address;
	}

	void* alloc_aligned(u64 size, u32 align) {
		size = align(size, 16);
		align = align(align, 16);

		Link* l = first;
		u64 address = heap_base;
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
					Link* freelink = create_link();
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
				allocate_into_free(l, size, alignfix);

				break;
			}

			address += l->length;
			l = l->next;
		}

		#ifdef TRACE_ALLOCS
		report("alloc aligned %llx (%x)\n", size, align);
		#endif

		return (void*)address;
	}

	void* realloc(void* ptr, u64 newsize) {
		if (!ptr)
			return vmm::alloc(newsize);

		Link* i = first;
		u64 addr = heap_base;
		u64 oldsize = 0;
		newsize = align(newsize, 16);

		while (i) {
			if (addr == (u64)ptr) {
				oldsize = i->length;
				if (newsize <= oldsize)
					return ptr;

				if (i->next) {
					if (i->next->length == newsize - i->length) {
						i->next->length -= newsize - i->length;
						i->length += newsize - i->length;

						// i->next törlése
						Link* old = i->next;
						i->next = old->next;
						old->next->prev = i;
						delete_link(old);

						return ptr;
					} else if (i->next->length > newsize - i->length) {
						i->next->length -= newsize - i->length;
						i->length += newsize - i->length;
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

		fatal("Elerhetetlen kod!\n");

whatever:
		void* newloc = vmm::alloc(newsize);
		memcpy(newloc, ptr, oldsize);
		free(ptr);
		return newloc;
	}

	u64 dump() {
		printk("===============================\n");
		Link* i = first;
		u64 addr = heap_base;
		while (i) {
			printk("[%p] %s: %08llx byte\n", (void*)addr, i->free ? "FREE" : "USED", i->length);
			addr += i->length;
			i = i->next;
		}
		printk("===============================\n");
		return addr - heap_base;
	}

	void free(void* p) {
		if (!p) return;

		u64 addr = heap_base;
		Link* i = first;

		while (i) {
			if (addr == (u64)p) break;

			addr += i->length;
			i = i->next;
		}

		if (!i || addr != (u64)p)
			fatal("Ervenytelen free!\n");

		if (i == (Link*)0x6966969696969669) {
			fatal("nem jo.. \n");
		}
		i->free = true;

		merge(i);
	}
}
