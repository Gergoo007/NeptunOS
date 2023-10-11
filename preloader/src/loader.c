#include <loader.h>

void load_kernel(void) {
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*) &_binary_out_kernel_start;
	if (strncmp((char*)ehdr->e_ident, "\177ELF", 4))
		printf("Invalid kernel magic!\n\r");
	else
		printf("Kernel detected successfully!\n\r");

	Elf64_Phdr* phdrs = (Elf64_Phdr*) ((u64)ehdr + ehdr->e_phoff);
	for (u8 i = 0; i < ehdr->e_phnum; i++) {
		Elf64_Phdr* phdr = (Elf64_Phdr*) ((u64)phdrs + ehdr->e_phentsize*i);
		if (phdr->p_type == PT_LOAD) {
			// Mindegyik page-et külön mappeljünk, ha már mappelve van akkor
			// ne írjuk felül a fizikai címet
			for (u8 j = 0; j < phdr->p_memsz/PAGESIZE+1; j++) {
				map_page(phdr->p_vaddr+(j*PAGESIZE), bm_get_free(), 0);
				// printf("Cím: %p\n\r", phdr->p_vaddr+(j*PAGESIZE) & ~(PAGESIZE));
			}

			// Vonjuk ki a szegmenst a kernel fileból
			// memcpy((u8*) ((u64)ehdr + phdr->p_offset), (u8*) (phdr->p_vaddr), phdr->p_memsz);
		}
	}
}
