#pragma once

#include <util/bitmap.h>

void vmm_init();
void* vmm_alloc();
void* vmm_reserve();
void vmm_free(void* a);
