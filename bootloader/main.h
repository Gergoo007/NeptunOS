#pragma once

#include "elf.h"

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

void validate_elf_file(Elf64_Ehdr* hdr);

void load_kernel(uintptr_t* entry);

void setup_gop(efi_gop_t** gop);

