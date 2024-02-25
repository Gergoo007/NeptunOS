#pragma once

#include <lib/int.h>

#include <graphics/console.h>

#pragma pack(1)

extern u32 ioapic_redirs[16];

typedef struct ioapic {
	u64 base;
	u64 gsi_base;
} ioapic_t;

typedef struct ioapic_redir_entry {
	union {
		struct {
			union {
				struct {
					u8 vector;
					u8 delivery : 3;
					u8 dest_mode : 1;
					u8 is_busy : 1;
					u8 active_low : 1;
					// Only works for lvl trig, if 1 then IOAPIC has received the int,
					// if 0 then it has received EOI
					u8 remote_irr : 1;
					u8 lvl_trig : 1;
					u8 mask : 1;
					u16 : 15;
				};
				u32 first_part;
			};

			union {
				struct {
					u32 : 24;
					u8 destination;
				};
				u32 second_part;
			};
		};

		u64 raw;
	};
} ioapic_redir_entry_t;

enum {
	IOAPIC_ID = 0x00,
	IOAPIC_REDIR = 0x01,
	IOAPIC_ARB_PRI = 0x02,
};

extern ioapic_t* ioapics;
extern u16 num_ioapics;

void ioapic_write(u16 reg, u32 val);
u32 ioapic_read(u16 reg);
void ioapic_write_entry(u32 gsi, u8 vector);
void ioapic_set_mask(u32 gsi, u8 mask);

__attribute__((unused)) inline static u32 irq_to_gsi(u8 irq) {
	return ioapic_redirs[irq];
}
