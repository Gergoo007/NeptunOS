#pragma once

#include <neptunos/libk/stdall.h>

typedef struct ioapic_id {
	u32 : 23;
	u8 id : 4;
	u8 : 5;
} _attr_packed ioapic_id_t;

typedef struct ioapic_version {
	union {
		struct {
			u8 ver;
			u8 _res1;
			u8 max_redirect_entries;
			u8 _res2;
		};
		u32 val;
	} _attr_packed;
} _attr_packed ioapic_version_t;

typedef struct ioapic_arb_pri {
	u32 _res1 : 23;
	u8 arb_pri : 4;
	u8 _res2 : 5;
} _attr_packed ioapic_arb_pri_t;

typedef struct ioapic_redirect_entry {
	union {
		struct {
			u8 vector;
			u8 delivery_mode : 3;
			u8 logical_dest : 1;
			u8 int_pending : 1;
			u8 active_low : 1;
			u8 eoi_received : 1;
			u8 lvl_triggered : 1;
			u8 mask : 1;
			u16 : 15;
		};
		u32 first_part;
	};
	union {
		struct {
			u64 : 24;
			u8 apic_id : 4;
			u8 cpu_set : 4;
		};
		u32 second_part;
	};
} _attr_packed ioapic_redirect_entry_t;

typedef struct ioapic {
	void* addr;
	u32 gis_base;
	u8 ioapic_id;
} ioapic_t;

typedef struct ioapic_override {
	u8 irq;
	u32 gsi;
} ioapic_override_t;
