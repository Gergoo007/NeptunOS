#include <loader.h>

// 2 MiB bőven elég lesz a kernelnek PLUSZ még az strtab-nak meg a symtab-nak is
u64 load_kernel(void) {
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)&_binary_out_kernel_start;
	
	if (strncmp("\177ELF", (char*)ehdr->e_ident, 4)) {
		printf("Kernel formátuma érvénytelen!\n\r");
		return 0;
	}

	Elf64_Phdr* phdr = (Elf64_Phdr*) ((u64)ehdr + ehdr->e_phoff);
	u64 kpage = bm_get_free();
	map_page(phdr->p_vaddr, kpage);

	// Összes segment-nél megnézi az offset + size-t,
	// ez fogja tartalmazni a legnagyobbat (utolsó szegmens végét)
	u64 k_end = 0;

	for (u8 i = 0; i < ehdr->e_phnum; i++, phdr++) {
		if (k_end < phdr->p_vaddr+phdr->p_memsz) k_end = phdr->p_vaddr+phdr->p_memsz;

		if (phdr->p_type != PT_LOAD)
			continue;

		if (phdr->p_memsz > phdr->p_filesz)
			memset((u8*)phdr->p_vaddr+phdr->p_filesz, 0, phdr->p_memsz-phdr->p_filesz);

		memcpy((u8*)ehdr + phdr->p_offset, (u8*)phdr->p_vaddr, phdr->p_filesz);
	}

	// Egy byte-ot kihagyok nehogy beleírjak az utolsó szegmensbe
	k_end++;

	u64 strtab1 = 0;
	u64 strtab2 = 0;
	u64 symtab = 0;

	Elf64_Shdr* shdr = (Elf64_Shdr*) ((u64)ehdr + ehdr->e_shoff);
	for (u8 i = 0; i < ehdr->e_shnum; i++, shdr++) {
		switch (shdr->sh_type) {
			case SHT_SYMTAB: {
				symtab = k_end;
				memcpy((u8*)ehdr + shdr->sh_offset, (u8*)k_end, shdr->sh_size);
				k_end += shdr->sh_size;

				u16 num_syms = shdr->sh_size / shdr->sh_entsize;
				kinfo.symtab_entries = num_syms;
				break;
			}

			case SHT_STRTAB: {
				if (strtab1)
					strtab2 = k_end;
				else
					strtab1 = k_end;

				memcpy((u8*)ehdr + shdr->sh_offset, (u8*)k_end, shdr->sh_size);
				k_end += shdr->sh_size;
				break;
			}
		}
	}

	kinfo.symtab = symtab;
	kinfo.strtab1 = strtab1;
	kinfo.strtab2 = strtab2;

	return ehdr->e_entry;
}
