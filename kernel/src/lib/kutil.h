#pragma once

#include "efi/boot_env.h"

#include "globals.h"
#include "graphics/graphics.h"

#include "graphics/text_renderer.h"
#include "serial/serial.h"
#include "libk/string.h"

#include "memory/memory.h"
#include "memory/efi_memory.h"
#include "memory/paging.h"

#include "tables/gdt.h"
#include "tables/idt.h"
#include "tables/interrupts.h"

#include "power/power.h"

extern idtr idt;

void kinit(system_info* _info);
void int_prep();
void setup_paging();

extern volatile unsigned char _binary_font_psf_start;

