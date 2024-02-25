#pragma once

#include <lib/int.h>
#include <graphics/console.h>

#include <acpi/madt.h>

void lapic_init();
void lapic_write_reg(u32 offset, u32 value);
u32 lapic_read_reg(u32 offset);
