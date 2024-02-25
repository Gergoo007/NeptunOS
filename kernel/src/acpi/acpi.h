#pragma once

#include <lib/multiboot.h>

#include <acpi/xsdt.h>
#include <acpi/madt.h>

#include <graphics/console.h>

#include <apic/lapic.h>
#include <apic/irqs.h>

#pragma pack(1)

void acpi_init(void* tag);
sdt_base_t* acpi_get_table(const char* name);
