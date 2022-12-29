#pragma once

#include "efi/boot_env.h"
#include "memory/memory.h"
#include <stddef.h>

extern uint16_t cursor_x;
extern uint16_t cursor_y;

extern system_info* info;

extern bitmap* bm;
extern void* mem_base;

extern void* KSTART;
extern void* KEND;

extern void __stack_chk_fail();
