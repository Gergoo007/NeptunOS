#pragma once

#include <neptunos/libk/stdall.h>
#include "apic.h"

typedef struct lapic_ver_reg {
	u8 ver;
	u8 res;
	u8 max_lvt_entry;
	u8 res2;
} lapic_ver_reg_t;

typedef struct lvt_entry {
	u8 vector_num;
	u8 nmi : 3;
	u8 : 1;
	u8 int_pending : 1;
	u8 active_low : 1;
	u8 remote_irr : 1;
	u8 lvl_triggered : 1;
	u8 mask : 1;
	u8 timer_mode : 2;
	u16 : 13;
} lvt_entry_t;

typedef struct lapic_regs {
	_Alignas(16) u32 _res1;
	_Alignas(16) u32 _res2;
	_Alignas(16) u32 lapic_id;
	_Alignas(16) lapic_ver_reg_t lapic_ver;
	_Alignas(16) u32 _res3[0x10];
	_Alignas(16) u32 tpr;
	_Alignas(16) u32 apr;
	_Alignas(16) u32 ppr;
	_Alignas(16) u32 eoi;
	_Alignas(16) u32 rrd;
	_Alignas(16) u32 ld;
	_Alignas(16) u32 dest_fmt;
	_Alignas(16) u32 spurious_int_vector;
	_Alignas(16) u32 in_service[0x20];
	_Alignas(16) u32 tmr[0x20];
	_Alignas(16) u32 irr[0x20];
	_Alignas(16) u32 err_status;
	_Alignas(16) u32 _res4[0x18];
	_Alignas(16) u32 lvt_cmci;
	_Alignas(16) u32 icr[0x08];
	_Alignas(16) lvt_entry_t lvt_timer;
	_Alignas(16) lvt_entry_t lvt_thermal;
	_Alignas(16) lvt_entry_t lvt_perf_monitoring_counters;
	_Alignas(16) lvt_entry_t lvt_lint0;
	_Alignas(16) lvt_entry_t lvt_lint1;
	_Alignas(16) lvt_entry_t lvt_error;
	_Alignas(16) u32 timer_initial_count;
	_Alignas(16) u32 timer_current_count;
	_Alignas(16) u32 _res5[0x10];
	_Alignas(16) u32 timer_divide_config;
	_Alignas(16) u32 _res6;
} _attr_packed lapic_regs_t;

void lapic_init(cpu_core_t* cpu);
