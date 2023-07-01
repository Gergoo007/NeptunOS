#pragma once

#include <neptunos/acpi/acpi.h>
#include <neptunos/cpuid/cpuid.h>
#include <neptunos/memory/paging.h>
#include <neptunos/interrupts/irqs.h>

typedef struct cpu_core_t {
	u8 cpu_id;
	u8 apic_id;
	void* lapic_base;
	u8 nmi_pin : 1;
	u8 cpu_enabled : 1;
	u8 online_capable : 1;
} _attr_packed cpu_core_t;

#include "lapic.h"

extern u16 num_cpus;
extern cpu_core_t* cpus;
extern void* lapic_base;

void apic_init(void);
