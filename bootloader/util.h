#pragma once

#include "elf.h"

#define check() \
	if(errno != 0)  { \
		switch (errno) { \
			case ENOMEM: \
				printf("Error: Out of memory"); \
				break; \
			case ENOENT: \
				printf("Error: No such file or directory"); \
				break; \
			case EIO: \
				printf("Error: I/O error"); \
				break; \
			case EFAULT: \
				printf("Error: Bad address"); \
				break; \
			default: \
				printf("Unhandled error: %d", errno); \
				break; \
		} \
		printf(" (L%d, file %s)\n", __LINE__, __FILE__); \
	} \

#define efi_check(s) if(EFI_ERROR(status)) \
	printf("Error: %s (L%d, file %s)\n", s, __LINE__, __FILE__); \
	status = EFI_SUCCESS; \

typedef struct memory_info {
	efi_memory_descriptor_t* 	mmap;
	size_t 						mmap_size;
	size_t 						mmap_key;
	size_t 						desc_size;
	void*						rsdp;
} __attribute__((packed)) memory_info;

typedef struct system_info {
	memory_info* 	mem_info;
	efi_gop_mode_t* g_info;
} __attribute__((packed)) system_info;

typedef void __attribute__((sysv_abi)) (*entry_point)(system_info*);

void validate_elf_file(Elf64_Ehdr* hdr);

void load_kernel(uintptr_t* entry);

void setup_gop(efi_gop_t** gop);
