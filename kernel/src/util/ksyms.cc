#include <util/ksyms.hh>
#include <util/string.hh>
#include <arch/limine.hh>

Elf64_Sym* ksyms;
u32 num_ksyms;

char* kstrtab;
char* kshstrtab;

__attribute__((section(".limine_requests"), used))
static volatile limine_executable_file_request file_req {
	.id = LIMINE_EXECUTABLE_FILE_REQUEST,
	.revision = 0,
	.response = nullptr,
};

void ksyms_read() {
	assert(file_req.response);
	limine_file* kernel = file_req.response->executable_file;
	assert(kernel);
	assert(kernel->address);
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)kernel->address;
	assert(ehdr);
	assert(!strncmp((char*)ehdr->e_ident, "\177ELF", 4));

	Elf64_Shdr* shs = (Elf64_Shdr*)((u64)ehdr + ehdr->e_shoff);
	kshstrtab = (char*)ehdr + shs[ehdr->e_shstrndx].sh_offset;
	for (u32 i = 0; i < ehdr->e_shnum; i++) {
		if (!strncmp(kshstrtab + shs[i].sh_name, ".strtab", 7)) {
			kstrtab = (char*)ehdr + shs[i].sh_offset;
		} else if (shs[i].sh_type == SHT_SYMTAB) {
			ksyms = (Elf64_Sym*)((u64)ehdr + shs[i].sh_offset);
			num_ksyms = shs[i].sh_size / sizeof(Elf64_Sym);
		}
	}

	// for (u32 i = 0; i < num_ksyms; i++)
	// 	printk("turi ip %s", strtab + ksyms[i].st_name);
}
