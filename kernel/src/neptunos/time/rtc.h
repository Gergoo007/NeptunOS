#pragma once

#include <neptunos/interrupts/nmi.h>

typedef struct rtc_reg_a {
	union {
		struct {
			u8 rs : 4;
			u8 dv : 3;
			u8 uip : 1;
		};
		u8 val;
	};
} _attr_packed rtc_reg_a_t;

typedef struct rtc_reg_b {
	union {
		struct {
			u8 dse : 1;
			u8 twenty_four_hr : 1;
			u8 is_binary : 1;
			u8 sqw_enable : 1;
			u8 int_on_update_end : 1;
			u8 alarm_int : 1;
			u8 periodic_int : 1;
			u8 set : 1;
		};
		u8 val;
	};
} _attr_packed rtc_reg_b_t;

typedef struct rtc_reg_c {
	union {
		struct {
			u8 _res : 4;
			u8 update_ended : 1;
			u8 alarm_expired : 1;
			u8 periodic_int_fired : 1;
			u8 irqf : 1;
		};
		u8 val;
	};
} _attr_packed rtc_reg_c_t;

typedef struct rtc_reg_d {
	union {
		struct {
			u8 _res : 7;
			u8 valid : 1;
		};
		u8 val;
	};
} _attr_packed rtc_reg_d_t;

void enable_rtc(void);
