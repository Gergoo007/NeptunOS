#include <loader.h>

u64 load_kernel(void) {
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)&_binary_out_kernel_start;
	
	if (strncmp("\177ELF", (char*)ehdr->e_ident, 4)) {
		printf("Kernel formátuma érvénytelen!\n\r");
		return 0;
	}

	Elf64_Phdr* phdr = (Elf64_Phdr*) ((u64)ehdr + ehdr->e_phoff);
	u64 kpage = bm_get_free();
	map_page(phdr->p_vaddr, kpage);

	for (u8 i = 0; i < ehdr->e_phnum; i++, phdr++) {
		if (phdr->p_type != PT_LOAD)
			continue;

		// TODO: több page kell majd egyszer a kernelnek
		// for (u64 addr = phdr->p_vaddr; addr < (phdr->p_vaddr+phdr->p_memsz); addr += PAGESIZE) {
		// 	u64 page = bm_get_free();
		// 	printf("%p -> %p\n\r", addr, page);
		// 	map_page(addr, page);
		// }

		if (phdr->p_memsz > phdr->p_filesz)
			memset((u8*)phdr->p_vaddr+phdr->p_filesz, 0, phdr->p_memsz-phdr->p_filesz);

		memcpy((u8*)ehdr + phdr->p_offset, (u8*)phdr->p_vaddr, phdr->p_filesz);
	}

	return ehdr->e_entry;
}
