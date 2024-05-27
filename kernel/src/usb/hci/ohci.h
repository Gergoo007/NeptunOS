#pragma once

#pragma pack(1)

#include <lib/types.h>

#include <pci/pci.h>

typedef volatile struct {
	u32 revision;
	u32 control;
	u32 cmd_sts;
	u32 int_sts;
	u32 int_enable;
	u32 int_disable;
	u32 hcca;
	u32 per_current_ed;
	u32 ctrl_head_ed;
	u32 ctrl_current_ed;
	u32 bulk_head_ed;
	u32 bulk_current_ed;
	u32 done_head;
	u32 fm_interv;
	u32 fm_remaining;
	u32 fm_periodic_start;
	u32 ls_threshold;
	u32 rh_desc_a;
	u32 rh_desc_b;
	u32 rh_sts;
	u32 rh_port_sts;
} ohci_regs;

typedef struct {
	u32 mmio;
} ohci_t;

enum {
	OHCI_ED_DIR_OUT = 0b01,
	OHCI_ED_DIR_IN = 0b10,
	OHCI_ED_DIR_TD = 0b00,
};

enum {
	OHCI_PID_SETUP = 0b00,
	OHCI_PID_OUT = 0b01,
	OHCI_PID_IN = 0b10,
};

typedef struct {
	struct {
		u8 addr : 7;
		u8 endp : 4;
		u8 direction : 2;
		u8 ls : 1;
		u8 skip : 1;
		u8 format : 1; // 1 ha isochronous, 0 különben
		u8 max_packet_size;
		u8 : 5;
	};
	u32 tail;
	union {
		u32 head;
		struct {
			u8 halt : 1;
			u8 c : 1;
			u8 : 2;
		};
	};
	u32 next_ed;
} ohci_ed_t;

typedef volatile struct {
	struct {
		u32 : 18;
		u8 buf_rounding : 1;
		u8 direction : 2;
		u8 int_delay : 3;
		u8 data_toggle : 2;
		u8 num_errors : 2;
		u8 status : 4;
	};
	u32 current_buf;
	u32 next_td;
	u32 buf_end;
} ohci_td_t;

typedef volatile struct ohci_hcca {
	u32 int_table[32];
	u16 frame_num;
	u16 pad1;
	u32 done_head;
} ohci_hcca;

extern ohci_t* ohcis;
extern u64 num_ohcis;

void ohci_init_controller(pci_hdr_t* device);
void ohci_send_in(ohci_t* hc, usb_dev_t* dev, u8* packet, void* output, u8 len);
void ohci_send_address(ohci_t* hc, usb_dev_t* dev, u8 addr);
