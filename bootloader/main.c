#include "main.h"

int main(int argc, char** argv) {
	efi_memory_descriptor_t* desc = NULL;
	efi_gop_t* gop = NULL;
	uintn_t map_size, map_key, desc_size;
	system_info* boot_struct;
	memory_info* mem_inf;
	call_kernel kmain;
	uintptr_t entry = 0;
	efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	uintn_t isiz = sizeof(efi_gop_mode_info_t);
	efi_gop_mode_info_t* mode;
	uint16_t i = 0;

	BS->LocateProtocol(&gop_guid, NULL, (void**)&gop);
	
	for (i = 0; i < gop->Mode->MaxMode; i++) {
		gop->QueryMode(gop, i, &isiz, &mode);
		if(mode->HorizontalResolution == 1280 && mode->VerticalResolution == 720)
			gop->SetMode(gop, i);
	}

	BS->GetMemoryMap(&map_size, desc, &map_key, &desc_size, NULL);
	map_size += 4 * desc_size;
	desc = (efi_memory_descriptor_t*)malloc(map_size);
	BS->GetMemoryMap(&map_size, desc, &map_key, &desc_size, NULL);

	boot_struct = (system_info*)malloc(sizeof(system_info));
	mem_inf = (memory_info*)malloc(sizeof(system_info));

	printf("Setting up GOP...\n");
	setup_gop(&gop);

	boot_struct->g_info = gop->Mode;
	boot_struct->mem_info = mem_inf;

	boot_struct->mem_info->mmap = desc;
	boot_struct->mem_info->mmap_size = map_size;
	boot_struct->mem_info->mmap_key = map_key;
	boot_struct->mem_info->desc_size = desc_size;

	load_kernel(&entry);
	kmain = (call_kernel)entry;
	printf("Resolution: %d x %d\n", boot_struct->g_info->Information->HorizontalResolution, boot_struct->g_info->Information->VerticalResolution);
	printf("...\n");

	/* Quit the bullshit. */
	exit_bs();

	kmain(boot_struct);

	while (1);
	(void)argc;
	(void)argv;
	return 0;
}
