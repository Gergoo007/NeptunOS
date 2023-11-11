#pragma once

#include <memory/heap/pmm.h>

#include <lib/bitmap.h>

void vmm_init(void);
void* request_page(void);
void* vmalloc(void);
