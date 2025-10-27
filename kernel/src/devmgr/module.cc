#include <devmgr/module.hh>
#include <arch/limine.hh>
#include <util/ustar.hh>
#include <util/string.hh>
#include <util/ksyms.hh>
#include <arch/amd64/paging.hh>

#define MODULES_BASE 0xfffffff000000000

__attribute__((section(".limine_requests"), used))
static volatile limine_module_request module_req {
	.id = LIMINE_MODULE_REQUEST,
	.revision = 1,
	.response = nullptr,

	.internal_module_count = 0,
	.internal_modules = nullptr,
};

namespace modules {
	Vector<Module> modules;
	// 64 modul van max, így mindegyiknek jut 1 GiB
	Bitmap bm(64);

	u64 link(void* a, u64 size) {
		Elf64_Ehdr* ehdr = (Elf64_Ehdr*)a;
		
		Elf64_Shdr* shdrs = (Elf64_Shdr*)((u64)a + ehdr->e_shoff);
		// .strtab, .shstrtab
		char* shstrtab = (char*)((u64)a + shdrs[ehdr->e_shstrndx].sh_offset);
		char* strtab;
		Elf64_Sym* symtab;
		u32 num_syms;
		for (u32 i = 0; i < ehdr->e_shnum; i++) {
			if (!strcmp(shstrtab + shdrs[i].sh_name, ".strtab")) {
				strtab = (char*)((u64)a + shdrs[i].sh_offset);
			} else if (shdrs[i].sh_type == SHT_SYMTAB) {
				symtab = (Elf64_Sym*)((u64)a + shdrs[i].sh_offset);
				num_syms = shdrs[i].sh_size / sizeof(Elf64_Sym);
			}
		}

		u64 entry = 0;
		constexpr const char* entryname = "_Z8mod_mainv";
		for (u32 i = 0; i < num_syms; i++) {
			if (!strcmp(strtab + symtab[i].st_name, entryname)) {
				entry = shdrs[symtab[i].st_shndx].sh_addr + symtab[i].st_value;
			}
		}
		if (!entry)
			fatal("Entry point not found as '%s'! Is the mangling correct?\n", entryname);

		// Végigloopolás az összes relocation-ön
		for (u32 i = 0; i < ehdr->e_shnum; i++) {
			assert(shdrs[i].sh_type != SHT_REL);
			if (shdrs[i].sh_type != SHT_RELA) continue;

			Elf64_Shdr& linkee = shdrs[shdrs[i].sh_info];
			Elf64_Rela* rela = (Elf64_Rela*)((u64)a + shdrs[i].sh_offset);
			u32 num_rela = shdrs[i].sh_size / sizeof(Elf64_Rela);
			for (u32 j = 0; j < num_rela; j++) {
				Elf64_Sym& sym = symtab[ELF64_R_SYM(rela[j].r_info)];
				u64 address = 0;

				// A kernelben keresendő-e a szimbólum?
				if (sym.st_shndx == SHN_UNDEF) {
					if (sym.st_info == STT_SECTION) {
						// Kernel található section
						fatal("Modul kernel section-t kert: %s\n", shstrtab + shdrs[sym.st_shndx].sh_name);
					} else {
						// Kernelben található szimbólum
						for (u32 k = 0; k < num_ksyms; k++) {
							if (!strcmp(kstrtab + ksyms[k].st_name, strtab + sym.st_name)) {
								address = ksyms[k].st_value;
							}
						}
					}
				} else {
					if (sym.st_info == STT_SECTION) {
						// Modulban található section
						address = shdrs[sym.st_shndx].sh_addr;
					} else {
						// Modulban található szimbólum
						for (u32 k = 0; k < num_syms; k++) {
							if (!strcmp(strtab + symtab[k].st_name, strtab + sym.st_name)) {
								address = shdrs[symtab[k].st_shndx].sh_addr + symtab[k].st_value;
							}
						}
					}
				}

				// Ide kell majd kiírni a memóriacímeket
				u64 mem = linkee.sh_addr;

				// thx 2 https://gist.github.com/DtxdF/e6d940271e0efca7e0e2977723aec360
				// Values:
				// A: This means the addend used to compute the value of the relocatable field.
				// B: This means the base address at which a shared object has been loaded into memory during execution. Generally, a shared object file is built with a 0 base virtual address, but the execution address will be different.
				// G: This means the offset into the global offset table at which the address of the relocation entry’s symbol will reside during execution.
				// GOT: This means the address of the global offset table.
				// L: This means the place (section offset or address) of the procedure linkage table entry for a symbol. A procedure linkage table entry redirects a function call to the proper destination. The link editor builds the initial procedure linkage table, and the dynamic linker modifies the entries during execution.
				// P: This means the place (section offset or address) of the storage unit being relocated (computed using r_offset).
				// S: This means the value of the symbol whose index resides in the relocation entry.
				switch (ELF64_R_TYPE(rela[j].r_info)) {
					// QWORD, S + A
					case R_X86_64_64: {
						*(u64*)(mem + rela[j].r_offset) = address + rela[j].r_addend;
						break;
					}
					default: {
						fatal("Unsupported reloc type: %lld\n", ELF64_R_TYPE(rela[j].r_info));
						break;
					}
				}
			}
		}

		return entry;
	}

	void load(void* a, u64 size) {
		// Ehhez a címhez képest lesznek a section-ök elhelyezve
		u64 modid = bm.find_and_set();
		u64 modbase = MODULES_BASE + gib2bytes(1) * modid;
		u64 sectionbase = modbase;
		
		Elf64_Ehdr* ehdr = (Elf64_Ehdr*)a;
		Elf64_Shdr* shdrs = (Elf64_Shdr*)((u64)a + ehdr->e_shoff);
		char* shstrtab = (char*)((u64)a + shdrs[ehdr->e_shstrndx].sh_offset);
		for (u32 i = 0; i < ehdr->e_shnum; i++) {
			if ((shdrs[i].sh_flags & SHF_ALLOC) && shdrs[i].sh_size) {
				u64 physbase = 0;

				// Hogy linkelés közben vissza lehessen olvasni anélkül
				// hogy eltárolnám valami hash table-ben
				shdrs[i].sh_addr = sectionbase;
				
				for (u32 j = 0; j < shdrs[i].sh_size; j += pmm::pagesize) {
					// Már mappelve van, valszeg az előző section miatt
					if (arch::paging_lookup(sectionbase + j) != -1ULL)
						continue;

					arch::map_page(
						sectionbase + j,
						(u64)PHYSICAL(pmm::alloc()),
						arch::MFLAGS::KDATA | arch::MFLAGS::EXE | arch::MFLAGS::s2M
					);
				}

				if (shdrs[i].sh_type == SHT_NOBITS) {
					memset(
						(void*)sectionbase,
						0,
						shdrs[i].sh_size
					);
				} else {
					memcpy(
						(void*)sectionbase,
						(void*)((u64)a + shdrs[i].sh_offset),
						shdrs[i].sh_size
					);
				}

				sectionbase += align(shdrs[i].sh_size, 32);
			}
		}

		void (*entry)() = (void (*)()) link(a, size);

		// Futtatás a mod_main() által
		entry();
	}

	void register_all() {
		auto* r = module_req.response;
		if (!r) {
			warn("No modules found!\n");
			return;
		}

		report("num mods: %lld\n", r->module_count);
		for (u32 i = 0; i < r->module_count; i++) {
			if (!strcmp(r->modules[i]->path, "/modules.tar")) {
				arch::check_pages((u64)r->modules[i]->address, r->modules[i]->size);
				auto files = ustar::list(r->modules[i]->address, r->modules[i]->size);
				for (const auto& f : files) {
					report("found module: %s; %p\n", f.name, f.address);
					load(f.address, f.size);
				}
			}
		}
	}
}
