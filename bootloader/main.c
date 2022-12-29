#include "elf.h"

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

typedef struct memory_info {
	efi_memory_descriptor_t* 	mmap;
	size_t 						mmap_size;
	size_t 						mmap_key;
	size_t 						desc_size;
} __attribute__((packed)) memory_info;

typedef struct system_info {
	memory_info* 	mem_info;
	efi_gop_mode_t* g_info;
} __attribute__((packed)) system_info;

typedef void __attribute__((sysv_abi)) (*call_kernel)(system_info*);

void validate_elf_file(Elf64_Ehdr* hdr) {
	if(	hdr->e_ident[EI_CLASS] == ELFCLASS64 && hdr->e_ident[EI_DATA] == ELFDATA2LSB &&
		hdr->e_type == ET_EXEC && hdr->e_phnum > 0 && 
		!memcmp("\177ELF", hdr->e_ident, 4)) {
			printf("This file will do.\n");
	} else {
			printf("Wrong ELF file properties!\n");	
	}
}

int main(int argc, char** argv) {
	efi_memory_descriptor_t* desc = NULL;
	uintn_t map_size, map_key, desc_size;
	efi_gop_t* gop = NULL;
	efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	system_info* boot_struct;
	memory_info* mem_inf;

	call_kernel kmain;

	Elf64_Ehdr* ehdr = NULL;
	Elf64_Phdr* phdr = NULL;
	uintptr_t entry;
	uint16_t i = 0;
	FILE* f;
	uint64_t size = 0;
	void* fptr = NULL;
	uintn_t isiz = sizeof(efi_gop_mode_info_t);
	efi_gop_mode_info_t* mode;

	BS->GetMemoryMap(&map_size, desc, &map_key, &desc_size, NULL);
	map_size += 4 * desc_size;
	desc = (efi_memory_descriptor_t*)malloc(map_size);
	BS->GetMemoryMap(&map_size, desc, &map_key, &desc_size, NULL);

	BS->LocateProtocol(&gop_guid, NULL, (void**)&gop);

	boot_struct = (system_info*)malloc(sizeof(system_info));
	mem_inf = (memory_info*)malloc(sizeof(system_info));

	boot_struct->g_info = gop->Mode;
	boot_struct->mem_info = mem_inf;
	
	for (i = 0; i < gop->Mode->MaxMode; i++) {
		gop->QueryMode(gop, i, &isiz, &mode);
		if(mode->HorizontalResolution == 1280 && mode->VerticalResolution == 720)
			gop->SetMode(gop, i);
	}

	boot_struct->mem_info->mmap = desc;
	boot_struct->mem_info->mmap_size = map_size;
	boot_struct->mem_info->mmap_key = map_key;
	boot_struct->mem_info->desc_size = desc_size;

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

	entry = ehdr->e_entry;

	free(fptr);
	free(ehdr);
	free(phdr);

	fclose(f);

	kmain = (call_kernel)entry;

	printf("Resolution: %d x %d\n", boot_struct->g_info->Information->HorizontalResolution, boot_struct->g_info->Information->VerticalResolution);
	printf("Starting kernel...\n");

	/* Quit the bullshit. */
	exit_bs();

	kmain(boot_struct);

	while (1);
	(void)argc;
	(void)argv;
	return 0;
}
