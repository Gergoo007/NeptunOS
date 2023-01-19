#include "util.h"

const char *types[] = {
	"EfiReservedMemoryType",
	"EfiLoaderCode",
	"EfiLoaderData",
	"EfiBootServicesCode",
	"EfiBootServicesData",
	"EfiRuntimeServicesCode",
	"EfiRuntimeServicesData",
	"EfiConventionalMemory",
	"EfiUnusableMemory",
	"EfiACPIReclaimMemory",
	"EfiACPIMemoryNVS",
	"EfiMemoryMappedIO",
	"EfiMemoryMappedIOPortSpace",
	"EfiPalCode"
};

void validate_elf_file(Elf64_Ehdr* hdr) {
	if(!(hdr->e_ident[EI_CLASS] == ELFCLASS64 && hdr->e_ident[EI_DATA] == ELFDATA2LSB 
	&& hdr->e_phnum > 0 && !memcmp("\177ELF", hdr->e_ident, 4))) {
		printf("Wrong ELF file properties!\n");	
	}
}

void setup_gop(efi_gop_t** gop) {
	efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	uintn_t isiz = sizeof(efi_gop_mode_info_t);
	efi_gop_mode_info_t* mode;
	uint16_t i = 0;

	BS->LocateProtocol(&gop_guid, NULL, (void**)&(*gop));
	
	for (i = 0; i < (*gop)->Mode->MaxMode; i++) {
		(*gop)->QueryMode(gop, i, &isiz, &mode);
		if(mode->HorizontalResolution == 1280 && mode->VerticalResolution == 720)
			(*gop)->SetMode(gop, i);
	}
}

void load_kernel(uintptr_t* entry) {
	Elf64_Ehdr* ehdr = NULL;
	Elf64_Phdr* phdr = NULL;
	FILE* f;
	uint64_t size = 0;
	void* fptr = NULL;
	uint16_t i = 0;

	if ((f = fopen("\\kernel", "r")) == NULL)
		printf("Error: failed to open kernel file!\n");

	fseek(f, 0, SEEK_END);
	if(!(size = ftell(f)))
		printf("Error: failed to get file size!\n");

	fseek(f, 0, SEEK_SET);
	check();
	fptr = (void*) malloc(size);
	check();
	fread(fptr, size, 1, f);
	check();

	ehdr = (Elf64_Ehdr*) fptr;

	validate_elf_file(ehdr);

	for(phdr = (Elf64_Phdr *)((uint8_t*)fptr + ehdr->e_phoff), i = 0;
		i < ehdr->e_phnum;
		i++, phdr = (Elf64_Phdr *)((uint8_t *)phdr + ehdr->e_phentsize)) {
			if(phdr->p_type == PT_LOAD) {
				// Use physical address as memory is identity mapped by UEFI
				// Switch to virtual addresses when paging is achieved in the bootloader
				memcpy((void*)phdr->p_vaddr, (uint8_t*)fptr + phdr->p_offset, phdr->p_filesz);
				check();
				memset((void*)(phdr->p_vaddr + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
				check();
			}
		}

	*entry = ehdr->e_entry;

	free(fptr);
	check();
}
