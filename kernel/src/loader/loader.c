#include <loader/loader.h>

void* loader_static(void* elf) {
	Elf64_Ehdr* ehdr = elf;
	if (strncmp((char*)ehdr->e_ident, "\177ELF", 4)) {
		error("Nem ELF!");
		return 0;
	}
	if (ehdr->e_type != ET_EXEC) {
		error("Nem futtathato!");
		return 0;
	}

	Elf64_Phdr* phdr = elf + ehdr->e_phoff;
	for (u8 i = 0; i < ehdr->e_phnum; i++) {
		if (phdr->p_type != PT_LOAD) continue;
		memcpy(elf + phdr->p_offset, (void*)phdr->p_vaddr, phdr->p_filesz);
		phdr++;
	}

	return (void*)ehdr->e_entry;
}
