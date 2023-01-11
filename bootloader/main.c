#include "util.h"

int main(int argc, char** argv) {
	uintptr_t entry = 0;
	system_info* boot_struct;
	memory_info* mem_inf;
	
	// Put initialization in a block so that unneccessary variables
	// are removed from the stack before kernel is called
	{
		efi_status_t status;
		efi_memory_descriptor_t* desc = NULL;
		efi_gop_t* gop = NULL;
		uintn_t map_size, map_key, desc_size;
		efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
		uintn_t isiz = sizeof(efi_gop_mode_info_t);
		efi_gop_mode_info_t* mode;
		uint16_t i = 0;

		status = BS->LocateProtocol(&gop_guid, NULL, (void**)&gop);
		efi_check("Unable to locate GOP");
		
		for (i = 0; i < gop->Mode->MaxMode; i++) {
			status = gop->QueryMode(gop, i, &isiz, &mode);
			efi_check("Unable to query modes");
			
			if(mode->HorizontalResolution == 1280 && mode->VerticalResolution == 720) {
				status = gop->SetMode(gop, i);
				efi_check("Unable to set mode");
			}
		}

		status = BS->GetMemoryMap(&map_size, desc, &map_key, &desc_size, NULL);
		// efi_check("Unable to get memory map size"); // This will always throw an error
		map_size += 4 * desc_size;
		desc = (efi_memory_descriptor_t*)malloc(map_size);
		status = BS->GetMemoryMap(&map_size, desc, &map_key, &desc_size, NULL);
		efi_check("Unable to get memory map");

		boot_struct = (system_info*)malloc(sizeof(system_info));
		check();
		mem_inf = (memory_info*)malloc(sizeof(system_info));
		check();

		boot_struct->g_info = gop->Mode;
		boot_struct->mem_info = mem_inf;

		boot_struct->mem_info->mmap = desc;
		boot_struct->mem_info->mmap_size = map_size;
		boot_struct->mem_info->mmap_key = map_key;
		boot_struct->mem_info->desc_size = desc_size;

		load_kernel(&entry);

		printf("Exiting BS and launching kernel...\n");

		/* Quit the bullshit. */
		exit_bs();
	}

	((entry_point)entry)(boot_struct); // cast 'entry' variable to kmain function and call it

	while (1);
	(void)argc;
	(void)argv;
	return 0;
}
