#include "main.h"

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
	if(	hdr->e_ident[EI_CLASS] == ELFCLASS64 && hdr->e_ident[EI_DATA] == ELFDATA2LSB &&
		hdr->e_type == ET_EXEC && hdr->e_phnum > 0 && 
		!memcmp("\177ELF", hdr->e_ident, 4)) {
			printf("This file will do.\n");
	} else {
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
