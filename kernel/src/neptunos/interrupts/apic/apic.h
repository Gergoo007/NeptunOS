#pragma once

#include <neptunos/acpi/acpi.h>
#include <neptunos/cpuid/cpuid.h>

typedef struct cpu_core_t {
	u8 cpu_id;
	u8 apic_id;
	u8 cpu_enabled : 1;
	u8 online_capable : 1;
} _attr_packed cpu_core_t;

extern u16 num_cpus;
extern cpu_core_t* cpus;
extern void* lapic_base;

void apic_init(void);
