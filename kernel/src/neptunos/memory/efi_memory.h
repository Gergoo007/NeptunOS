#pragma once

#define EfiReservedMemoryType			0
#define EfiLoaderCode					1
#define EfiLoaderData					2
#define EfiBootServicesCode				3
#define EfiBootServicesData				4
#define EfiRuntimeServicesCode			5
#define EfiRuntimeServicesData			6
#define EfiConventionalMemory			7
#define EfiUnusableMemory				8
#define EfiACPIReclaimMemory			9
#define EfiACPIMemoryNVS				10
#define EfiMemoryMappedIO				11
#define EfiMemoryMappedIOPortSpace		12
#define EfiPalCode						13
#define EfiPersistentMemory				14

extern const char* EFI_MEMORY_TYPES[15];
