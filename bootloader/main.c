#include "paging.h"

#define HORIZ_PREFERENCE 1280
#define VERTI_PREFERENCE 720

void map_address(void* virtual_address, void* physical_address) {
	// Indexes
	// pt_i points to the page, not the page table
	uint16_t pt_i, pd_i, pdp_i, pml4_i;
	indexer(virtual_address, &pt_i, &pd_i, &pdp_i, &pml4_i);

	page_map_entry pde;

	efi_status_t status;

	pde = pml4->entries[pml4_i];
	page_map_level* pdp;
	if (!pde.present) {
		printf("PDP not present...\n");
		pdp = malloc(0x1000);
		pdp = (page_map_level*)((uint64_t)pdp - ((uint64_t)pdp % 0x1000));
		efi_check("Unable to allocate for pdp!");
		memset(pdp, 0, 0x1000);
		pde.address = (uint64_t)pdp >> 12;
		pde.present = 1;
		pde.read_write = 1;
		pml4->entries[pml4_i] = pde;
	} else {
		pdp = (page_map_level*)((uint64_t)pde.address << 12);
	}

	if(pde.present)
		printf("PDP is present now...\n");
	
	pde = pdp->entries[pdp_i];
	page_map_level* pd;
	if (!pde.present) {
		printf("PD not present...\n");
		pd = malloc(0x1000);
		efi_check("Unable to allocate for pd!");
		pd = (page_map_level*)((uint64_t)pd - ((uint64_t)pd % 0x1000));
		memset(pd, 0, 0x1000);
		pde.address = (uint64_t)pd >> 12;
		pde.present = 1;
		pde.read_write = 1;
		pdp->entries[pdp_i] = pde;
	} else {
		pd = (page_map_level*)((uint64_t)pde.address << 12);
	}

	if(pde.present)
		printf("PD is present now...\n");

	pde = pd->entries[pd_i];
	page_map_level* pt;
	//if (!pde.present) {
		printf("PT not present...\n");
		pt = malloc(0x1000);
		efi_check("Unable to allocate for pt!");
		memset(pt, 0, 0x1000);
		pde.address = (uint64_t)pt >> 12;
		pde.present = 1;
		pde.read_write = 1;
		pd->entries[pd_i] = pde;
	//} else {
	//	pt = (page_map_level*)((uint64_t)pde.address << 12);
	//}

	if(pde.present)
		printf("PT is present now...\n");
	
	printf("Addresses: %p %p %p\n", pt, pd, pdp);

	printf("Before\n");
	pde = pt->entries[pt_i];
	printf("Before\n");
	pde.address = ((uint64_t)physical_address >> 12);
	printf("Before\n");
	pde.present = 1;
	printf("Before\n");
	pde.read_write = 1;
	pt->entries[pt_i] = pde;
}

const char* EFI_MEMORY_TYPES[] = {
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
    "EfiPalCode",
};

typedef struct rsdp_desc {
	char signature[8];
	uint8_t checksum;
	char oemid[6];
	uint8_t rev;
	uint32_t rsdt_address;
} __attribute__ ((packed)) rsdp_desc;

typedef struct rsdp_desc_2 {
	rsdp_desc first;
	uint32_t length;
	uint64_t xsdt_address;
	uint8_t ext_checksum;
	uint8_t reserved[3];
} __attribute__ ((packed)) rsdp_desc_2;

uint8_t compare_guid(efi_guid_t* guid_1, efi_guid_t* guid_2) {
	int32_t *g1, *g2, r = 0;

	g1 = (int32_t*)guid_1;
	g2 = (int32_t*)guid_2;

	r  = g1[0] - g2[0];
	r |= g1[1] - g2[1];
	r |= g1[2] - g2[2];
	r |= g1[3] - g2[3];

	return r;
}

int main(int argc, char** argv) {
	uintptr_t entry = 0;
	system_info* boot_struct;
	memory_info* mem_inf;
	uint64_t total_mem = 0;
	uint8_t* addr;

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
			
			if(mode->HorizontalResolution == HORIZ_PREFERENCE && mode->VerticalResolution == VERTI_PREFERENCE) {
				status = gop->SetMode(gop, i);
				efi_check("Unable to set mode");
			}
		}

		BS->GetMemoryMap(&map_size, desc, &map_key, &desc_size, NULL);
		// efi_check("Unable to get memory map size"); // This will always throw an error
		map_size += 4 * desc_size;
		desc = (efi_memory_descriptor_t*)malloc(map_size);
		check();
		status = BS->GetMemoryMap(&map_size, desc, &map_key, &desc_size, NULL);
		efi_check("Unable to get memory map");

		for (efi_memory_descriptor_t* desc_i = desc; (uint64_t)desc_i < (uint64_t)desc + (uint64_t)map_size; desc_i = (efi_memory_descriptor_t*)((uint8_t*)desc_i + desc_size)) {
			total_mem += desc_i->NumberOfPages*0x1000;
			//printf("%s: %d KiB\n", EFI_MEMORY_TYPES[desc_i->Type], desc_i->NumberOfPages*0x1000/1024);
		}

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

		printf("Detected ram: %d MiB\n", total_mem / 1024 / 1024);

		efi_guid_t acpi_ver_1 = ACPI_TABLE_GUID;
		efi_guid_t acpi_ver_2 = ACPI_20_TABLE_GUID;

		void* rsdp = NULL;
		uint8_t ver;

		// Check for ACPI 2, if not present then ACPI 1
		// Go through each entry in the ConfigurationTable
		// and compare it's guid to the ACPI's
		for (uint16_t i = 0; i < ST->NumberOfTableEntries; i++) {
			if (!compare_guid(&acpi_ver_2, &ST->ConfigurationTable[i].VendorGuid)) {
				if (strncmp("RSD PTR ", (char*)&ST->ConfigurationTable->VendorTable, 8)) {
					rsdp = ST->ConfigurationTable[i].VendorTable;
					ver = 2;
					break;
				}
			}
		}

		if (!rsdp) {
			for (uint16_t i = 0; i < ST->NumberOfTableEntries; i++) {
				if (!compare_guid(&acpi_ver_1, &ST->ConfigurationTable[i].VendorGuid)) {
					if (strncmp("RSD PTR ", (char*)&ST->ConfigurationTable->VendorTable, 8)) {
						rsdp = ST->ConfigurationTable[i].VendorTable;
						ver = 1;
						break;
					}
				}
			}
		}

		if (!rsdp) {
			printf("Unable to find RSDP!\n");
			return 0;
		}

		if (ver == 2) {
			rsdp_desc_2* _desc = (rsdp_desc_2*)rsdp;
			boot_struct->mem_info->rsdp = _desc;
		} else if (ver == 1) {
			rsdp_desc* _desc = (rsdp_desc*)rsdp;
			boot_struct->mem_info->rsdp = _desc;
			printf("Warning: first version detected\n");
		}

		

		/* Quit the bullshit. */
		exit_bs();
	}
	
	((entry_point)entry)(boot_struct); // cast 'entry' variable to kmain function and call it

	while (1);
	(void)argc;
	(void)argv;
	return 0;
}
 