#pragma once

#include <lib/int.h>

typedef struct cpu_core {
	u8 acpi_cpu_id;
	u8 apic_id;
	u8 cpu_enabled : 1;
	u8 online_cap : 1;
} cpu_core_t;

extern u16 num_cores;
extern cpu_core_t* cores;
