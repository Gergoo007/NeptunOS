#include <loader/loader.hh>
#include <util/elf.hh>
#include <util/string.hh>
#include <scheduler/scheduler.hh>
#include <arch/amd64/paging.hh>
#include <mm/pmm.hh>

program::program(Span<u8> exefile) {
	// Új címtér létrehozása
	cr3 = (page_table_t*)kmalloc_aligned(0x1000, 0x1000);
	// A kernel címterének átmásolása (PML4 második fele)
	memcpy((void*)((u64)cr3 + 0x800), (void*)((u64)pml4 + 0x800), 0x800);

	// Az alsó 128 TiB a usermode-é lesz
	memset(cr3, 0, 0x800);

	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)(u8*)exefile;
	if (strncmp((char*)ehdr->e_ident, "\177ELF", 4)) fatal("Only ELF files are supported for loading!");

	Elf64_Phdr* phdrs = (Elf64_Phdr*)((u8*)exefile + ehdr->e_phoff);
	for (u32 i = 0; i < ehdr->e_phnum; i++) {
		if (phdrs[i].p_type != PT_LOAD) continue;

		// A PT_LOAD szegmensek nem lógnak egymásba, ez egyszerűbbé teszi a kódot,
		// bár 2 MiB-os egységeket használok így megintcsak szar lesz
		for (u64 mapped = 0; mapped < phdrs[i].p_memsz; mapped += 0x200000) {
			if (paging_lookup(cr3, phdrs[i].p_vaddr + mapped) == -1ull)
				map_page(cr3, phdrs[i].p_vaddr + mapped, (u64)PHYSICAL(pmm_alloc(PMM_PAGESIZE)), 0b11 | MFLAGS::EXE | MFLAGS::USER | PMM_PAGESIZE_FLAG);
		}

		asm volatile ("movq %0, %%cr3" :: "r"(paging_lookup(cr3)));

		memset((void*)phdrs[i].p_vaddr, 0, phdrs[i].p_memsz);
		memcpy((void*)phdrs[i].p_vaddr, (u8*)exefile + phdrs[i].p_offset, phdrs[i].p_filesz);

		asm volatile ("movq %0, %%cr3" :: "r"(paging_lookup(pml4)));

		// for (u64 mapped = 0; mapped < phdrs[i].p_memsz; mapped += PMM_PAGESIZE) {
		// 	// Milyen autista találta ki hogy az enum struct ne legyen castelhető? Fasz kivan
		// 	u64 flags = MFLAGS::USER | MFLAGS::PRESENT; // | MFLAGS::s2M;
		// 	if (phdrs[i].p_flags & PF_W)
		// 		flags |= MFLAGS::RW;
		// 	if (phdrs[i].p_flags & PF_X)
		// 		flags |= MFLAGS::EXE;

		// 	map_page(cr3, phdrs[i].p_vaddr + mapped, (u64)paging_lookup(phdrs[i].p_vaddr), flags);
		// }
	}

	constexpr u64 VSTACK = 0x00007fffff000000; // stack: 0x00007fffff200000 - 0x00007fffff000000

	u64 stackphys = PHYSICAL((u64)pmm_alloc(USER_STACK_SIZE));
	map_page(cr3, VSTACK, stackphys, MFLAGS::UDATA | MFLAGS::s2M); static_assert(USER_STACK_SIZE == P2M);

	stack = VSTACK + PMM_PAGESIZE - 256;
	cr3 = (page_table_t*)paging_lookup(cr3);
	entry = (decltype(entry))ehdr->e_entry;
}

program::~program() {
	pmm_free((void*)stack);
	
	// User page table-k visszaszedése
	// TODO: WATERMARK LECSERÉLÉSE
}

void program::launch() {
	sched_add_user_process((u64)cr3, stack, entry);
}
