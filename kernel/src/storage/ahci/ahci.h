#pragma once

#include <lib/types.h>
#include <lib/attrs.h>

#include <pci/pci.h>
#include <graphics/console.h>

enum {
	AHCI_FIS_REG_H2D	= 0x27,
	AHCI_FIS_REG_D2H	= 0x34,
	AHCI_FIS_DMA_ACT	= 0x39,
	AHCI_FIS_DMA_SETUP	= 0x41,
	AHCI_FIS_DATA		= 0x46,
	AHCI_FIS_BIST		= 0x58,
	AHCI_FIS_PIO_SETUP	= 0x5F,
	AHCI_FIS_DEV_BITS	= 0xA1,
};

enum {
	AHCI_SIG_SATA	= 0x00000101,
	AHCI_SIG_SATAPI	= 0xEB140101,
	AHCI_SIG_SEMB	= 0xC33C0101,	// Enclosure management bridge
	AHCI_SIG_PM		= 0x96690101,
};

enum {
	ATA_DEV_BUSY = 0x80,
	ATA_DEV_DRQ = 0x08,
};

typedef volatile struct _attr_packed {
	u8 type;

	u8 port_multi : 4;
	u8 : 3;
	u8 cmd_ctrl : 1;

	u8 cmd;
	u8 featurel;

	u8 lba0;
	u8 lba1;
	u8 lba2;
	u8 dev;

	u8 lba3;
	u8 lba4;
	u8 lba5;
	u8 featureh;

	u8 countl;
	u8 counth;

	u8 isochronous_complete;
	u8 ctrl;

	u32 : 32;
} fis_reg_h2d;

typedef volatile struct _attr_packed {
	u8 type;

	u8 port_multi : 4;
	u8 : 2;
	u8 intr : 1;
	u8 : 1;

	u8 sts;
	u8 err;

	u8 lba0;
	u8 lba1;
	u8 lba2;
	u8 dev;

	u8 lba3;
	u8 lba4;
	u8 lba5;
	u8 : 8;

	u8 countl;
	u8 counth;

	u16 : 16;

	u32 : 32;
} fis_reg_d2h;

typedef volatile struct _attr_packed {
	u32 cmd_listl;
	u32 cmd_listu;

	u32 fisl;
	u32 fisu;

	u32 int_sts;
	u32 int_enable;

	union {
		u32 cmd;
		struct {
			u8 start : 1;
			u8 spin_up : 1;
			u8 power_on : 1;
			u8 cmd_list_overr : 1;
			u8 fis_rx_enable : 1;
			u8 : 3;
			u8 current_cmd_slot : 5;
			u8 mpss : 1;
			u8 fis_rx_running : 1;
			u8 cmd_list_running : 1;
			u8 cps : 1;
			u8 pma : 1;
		};
	};
	u32 : 32;

	u32 task_file_data;
	u32 sig;

	union {
		u32 ssts;
		struct {
			u8 det : 4;
			u8 spd : 4;
			u8 ipm : 4;
		};
	};
	u32 sctrl;
	u32 serror;
	u32 sactive;

	u32 cmd_issue;

	u32 snotif;

	u32 fis_based_switch;

	u32 res[15];
} hba_port;

typedef volatile struct _attr_packed {
	u32 host_capability;
	u32 global_host_ctrl;
	u32 intr_sts;
	u32 ports_impl;
	u32 ver;
	u32 ccc_ctrl;
	u32 ccc_pts;
	u32 em_loc;
	u32 em_ctrl;
	u32 host_capability2;
	u32 handoff_sts;

	u8 res[0xd4];

	hba_port ports[0];
} hba_mem;

typedef volatile struct _attr_packed {
	u8 fis_len : 5; // FIS hossz * 32 bit
	u8 satapi : 1;
	u8 write : 1;
	u8 prefetchable : 1;
	u8 reset : 1;
	u8 bist : 1;
	u8 clear_busy_upon_ok : 1;
	u8 : 1;
	u8 pm : 1;

	u16 prdt_num_entries;
	
	volatile u32 prd_bytes_transferred;

	u32 cmd_table_desc_l;
	u32 cmd_table_desc_u;

	u32 _[4];
} hba_cmd_hdr;

typedef volatile struct _attr_packed {
	u32 data_base_addrl;
	u32 data_base_addru;
	u32 : 32;

	u32 num_bytes : 22;
	u32 : 9;
	u32 ioc : 1;
} hba_prdt_entry;

typedef volatile struct _attr_packed {
	u8 cmd_fis[64];
	u8 satapi_cmd[16];
	u8 _[48];

	hba_prdt_entry prdt_entry[0]; // Lehet belőle semennyi meg 65535 is
} hba_cmd_table;

typedef struct {
	u32 pi;
} ahci;

void ahci_init(pci_hdr_t* pci);
void ahci_read(hba_port* port, u64 start, u64 count, void* buf);