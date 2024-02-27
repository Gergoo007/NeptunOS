#pragma once

#include <lib/multiboot.h>

#include <acpi/xsdt.h>
#include <acpi/madt.h>
#include <acpi/mcfg.h>

#include <graphics/console.h>

#include <apic/apic.h>

#pragma pack(1)

void acpi_init(void* tag);
sdt_base_t* acpi_get_table(const char* name);
