#include "main.h"

void load_kernel(uintptr_t* entry) {
	Elf64_Ehdr* ehdr = NULL;
	Elf64_Phdr* phdr = NULL;
	FILE* f;
	uint64_t size = 0;
	void* fptr = NULL;
	uint16_t i = 0;

	if ((f = fopen("\\kernel", "r")) == NULL)
		printf("Failed to open kernel file!\n");

	fseek(f, 0, SEEK_END);
	if(!(size = ftell(f)))
		printf("Failed to get file size!\n");

	fseek(f, 0, SEEK_SET);
	fptr = (void*) malloc(size);
	fread(fptr, size, 1, f);

	ehdr = (Elf64_Ehdr*) fptr;

	validate_elf_file(ehdr);

	for(phdr = (Elf64_Phdr *)((uint8_t*)fptr + ehdr->e_phoff), i = 0;
		i < ehdr->e_phnum;
		i++, phdr = (Elf64_Phdr *)((uint8_t *)phdr + ehdr->e_phentsize)) {
			if(phdr->p_type == PT_LOAD) {
				printf("Found segment\n");
				memcpy((void*)phdr->p_vaddr, (uint8_t*)fptr + phdr->p_offset, phdr->p_filesz);
				memset((void*)(phdr->p_vaddr + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
			}
		}

	*entry = ehdr->e_entry;

	free(fptr);
	free(ehdr);
	free(phdr);

	fclose(f);
}
