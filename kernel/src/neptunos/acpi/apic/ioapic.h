#pragma once

#include <neptunos/libk/stdall.h>

typedef struct ioapic_id {
	u32 : 23;
	u8 id : 4;
	u8 : 5;
} _attr_packed ioapic_id_t;

typedef struct ioapic_version {
	u8 ver;
	u8 _res;
	u8 max_redirect_entries;
} _attr_packed ioapic_version_t;

typedef struct ioapic_arb_pri {
	u32 _res1 : 23;
	u8 arb_pri : 4;
	u8 _res2 : 5;
} _attr_packed ioapic_arb_pri_t;

typedef struct ioapic_redirect_entry {
	u8 vector;
	u8 delivery_mode : 4;
	u8 logical_dest : 1;
	u8 int_pending : 1;
	u8 active_low : 1;
	u8 eoi_received : 1;
	u8 lvl_triggered : 1;
	u8 mask : 1;
	u64 : 38;
	u8 apic_id : 4;
	u8 cpu_set : 4;
} _attr_packed ioapic_redirect_entry;

typedef struct ioapic_registers {
	ioapic_id_t id;
	ioapic_version_t ver;
	ioapic_arb_pri_t arb_pri;
} _attr_packed ioapic_registers_t;
