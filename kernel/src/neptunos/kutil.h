#pragma once

#include <neptunos/efi/boot_env.h>

#include <neptunos/globals.h>
#include <neptunos/graphics/graphics.h>

#include <neptunos/graphics/text_renderer.h>
#include <neptunos/serial/serial.h>
#include <neptunos/libk/string.h>

#include <neptunos/memory/memory.h>
#include <neptunos/memory/paging.h>

#include <neptunos/tables/gdt.h>
#include <neptunos/tables/idt.h>
#include <neptunos/tables/interrupts.h>

#include <neptunos/hw_interrupts/pic.h>
#include <neptunos/hw_interrupts/pit.h>

#include <neptunos/power/power.h>

#include <neptunos/acpi/acpi.h>

extern idtr idt;

void kinit();

extern volatile unsigned char _binary_font_psf_start;

