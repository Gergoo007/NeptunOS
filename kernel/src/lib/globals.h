#pragma once

#include "efi/bootinfo.h"
#include "memory/memory.h"

extern uint16_t cursor_x;
extern uint16_t cursor_y;

extern BootInfo* graphics;

extern bitmap* bm;
extern void* mem_base;

extern void* KSTART;
extern void* KEND;

extern void __stack_chk_fail();
