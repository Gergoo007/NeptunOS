#include <devmgr/module.hh>
#include <arch/limine.hh>
#include <util/ustar.hh>
#include <util/string.hh>
#include <arch/amd64/paging.hh>

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

	void link(void* a, u64 size) {
		Elf64_Ehdr* ehdr = (Elf64_Ehdr*)a;
		
		Elf64_Shdr* shdrs = (Elf64_Shdr*)((u64)a + ehdr->e_shoff);
		// .strtab, .shstrtab
		char* shstrtab = (char*)((u64)a + shdrs[ehdr->e_shstrndx].sh_offset);
		char* strtab;
		Elf64_Sym* symtab;
		// u32 num_syms;
		for (u32 i = 0; i < ehdr->e_shnum; i++) {
			if (!strcmp(shstrtab + shdrs[i].sh_name, ".strtab")) {
				strtab = (char*)((u64)a + shdrs[i].sh_offset);
			} else if (shdrs[i].sh_type == SHT_SYMTAB) {
				symtab = (Elf64_Sym*)((u64)a + shdrs[i].sh_offset);
				// num_syms = shdrs[i].sh_size / sizeof(Elf64_Sym);
			}
		}

		for (u32 i = 0; i < ehdr->e_shnum; i++) {
			if (!strncmp(".rela.", shstrtab + shdrs[i].sh_name, 6)) {
				// Itt egy .rela.* section, amit meg kell keresni és linkelni
				char* linkeename = shstrtab + shdrs[i].sh_name + 5;
				Elf64_Rela* rela = (Elf64_Rela*)((u64)a + shdrs[i].sh_offset);
				u32 num_rela = shdrs[i].sh_size / sizeof(Elf64_Rela);

				for (u32 j = 0; j < ehdr->e_shnum; j++) {
					if (!strcmp(shstrtab + shdrs[j].sh_name, linkeename)) {
						report("link this %s\n", shstrtab + shdrs[j].sh_name);

						for (u32 k = 0; k < num_rela; k++) {
							Elf64_Sym& sym = symtab[ELF64_R_SYM(rela[k].r_info)];
							report("relocate [type %llx]: %s\n", ELF64_R_TYPE(rela[k].r_info), strtab + sym.st_name);
						}
					}
				}
			}
		}
	}

	void load(void* a, u64 size) {
		link(a, size);

		// Futtatás a mod_main() által
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
