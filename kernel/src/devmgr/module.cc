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

vector<module_t> modules;
// 64 modul van max, így mindegyiknek jut 1 GiB
static bitmap_t bm;

u64 modules_link(void* a, u64 size) {
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
		fatal("Entry point not found as '%s'! Is the mangling correct?", entryname);

	// Végigloopolás az összes relocation-ön
	for (u32 i = 0; i < ehdr->e_shnum; i++) {
		if (shdrs[i].sh_type == SHT_REL)
			fatal("'%s' nevu section rel!", shstrtab + shdrs[i].sh_name);
		if (shdrs[i].sh_type != SHT_RELA) continue;

		Elf64_Shdr& linkee = shdrs[shdrs[i].sh_info];
		Elf64_Rela* rela = (Elf64_Rela*)((u64)a + shdrs[i].sh_offset);
		u32 num_rela = shdrs[i].sh_size / sizeof(Elf64_Rela);
		for (u32 j = 0; j < num_rela; j++) {
			Elf64_Sym& sym = symtab[ELF64_R_SYM(rela[j].r_info)];
			u64 address = 0;

			bool found = false;
			const char* name = (sym.st_info == STT_SECTION) ? (shstrtab + shdrs[sym.st_shndx].sh_name) : strtab + sym.st_name;

			// Meg kell oldani
			if (sym.st_shndx == SHN_UNDEF) {
				for (u32 k = 0; k < num_ksyms; k++) {
					if (!strcmp(kstrtab + ksyms[k].st_name, name)) {
						found = true;
						address = ksyms[k].st_value;
						break;
					}
				}
			} else {
				address = shdrs[sym.st_shndx].sh_addr + sym.st_value; // hozzá kell adni?
				found = true;
			}

			if (!found)
				fatal("Szimbolum nem talalhato: %s", name);

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
					fatal("Unsupported reloc type: %lld", ELF64_R_TYPE(rela[j].r_info));
					break;
				}
			}
		}
	}

	return entry;
}

void modules_load(const module_t& m) {
	void* a = m.content;
	u64 size = m.size;

	// Ehhez a címhez képest lesznek a section-ök elhelyezve
	u64 modid = bm.find_and_set();
	u64 modbase = MODULES_BASE + gib2bytes(1) * modid;
	u64 sectionbase = modbase;
	
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)a;
	Elf64_Shdr* shdrs = (Elf64_Shdr*)((u64)a + ehdr->e_shoff);
	for (u32 i = 0; i < ehdr->e_shnum; i++) {
		if ((shdrs[i].sh_flags & SHF_ALLOC) && shdrs[i].sh_size) {
			// Hogy linkelés közben vissza lehessen olvasni anélkül
			// hogy eltárolnám valami hash table-ben
			shdrs[i].sh_addr = sectionbase;
			
			for (u32 j = 0; j < shdrs[i].sh_size; j += pmm_pagesize) {
				// Már mappelve van, valszeg az előző section miatt
				if (paging_lookup(sectionbase + j) != -1ULL)
					continue;

				map_page(
					sectionbase + j,
					(u64)PHYSICAL(pmm_alloc()),
					MFLAGS::KDATA | MFLAGS::EXE | MFLAGS::s2M
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

	void (*entry)() = (void (*)()) modules_link(a, size);

	// Futtatás a mod_main() által
	entry();
}

void modules_register(void* a, u64 size, const char* modfilename) {
	Elf64_Ehdr* e = (Elf64_Ehdr*)a;
	// Ennek \127-nek kéne lennie, 2002 óta nem lett kijavítva ez az elf.h-ban
	assert(!strncmp("\177ELF", (char*)e->e_ident, 4));

	module_metadata_t* md = nullptr;
	Elf64_Shdr* shdrs = (Elf64_Shdr*) ((u64)a + e->e_shoff);
	const char* shstrtab = (char*) ((u64)a + shdrs[e->e_shstrndx].sh_offset);
	for (u32 i = 0; i < e->e_shnum; i++) {
		if (!strcmp(".modinfo", shstrtab + shdrs[i].sh_name)) {
			md = (module_metadata_t*) ((u64)a + shdrs[i].sh_offset);
			break;
		}
	}

	if (!md) {
		error("Invalid module (no .modinfo section found): %s", modfilename);
	} else {
		module_t mod { .content = a, .size = size, .metadata = md, };
		modules.emplace(mod);
		if (md->triggertype == ModuleTriggerTypes::ANY)
			modules_load(mod);
	}
}

void modules_register_all() {
	auto* r = module_req.response;
	if (!r) {
		warn("No modules found!");
		return;
	}

	report("num mods: %lld", r->module_count);
	for (u32 i = 0; i < r->module_count; i++) {
		if (!strcmp(r->modules[i]->path, "/modules.tar")) {
			check_pages((u64)r->modules[i]->address, r->modules[i]->size);
			auto files = ustar_list(r->modules[i]->address, r->modules[i]->size);

			for (const auto& f : files) {
				// Érvényes modul fájl?
				modules_register(f.address, f.size, f.name);
			}
		}
	}
}
