#pragma once

#include "../multiboot2.h"
#include "pmm.h"
#include "../serial.h"

#define NULL ((void*)0)

extern void* heap_base;
extern uint64_t heap_size;
extern uint64_t total;
void interpret_mmap(multiboot_tag_efi_mmap_t*);
