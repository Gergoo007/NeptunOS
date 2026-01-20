#pragma once

#include <types.hh>
#include <devmgr/devmgr.hh>

// struct ahci_reg {
// 	u32 offset;
// 	// size is always 4 bytes

// 	constexpr ahci_reg(u32 o): offset(o) {}

// 	u32 read(u64 mmio, u32 port = 0) const {
// 		if (offset > 0x100) {
// 			// Port reg
// 			return *(volatile u32*)(mmio + port * offset);
// 		} else {
// 			return *(volatile u32*)(mmio + offset);
// 		}
// 	}

// 	void write(u64 mmio, u32 val, u32 port) const {
// 		if (offset > 0x100) {
// 			// Port reg
// 			*(volatile u32*)(mmio + port * offset) = val;
// 		} else {
// 			*(volatile u32*)(mmio + offset) = val;
// 		}
// 	}
// };

// struct AhciRegs {
// 	static constexpr ahci_reg caps			{ 0x00 };
// 	static constexpr ahci_reg ghc			{ 0x04 };
// 	static constexpr ahci_reg int_sts		{ 0x08 };
// 	static constexpr ahci_reg port_impl		{ 0x0c };
// 	static constexpr ahci_reg version		{ 0x10 };
// 	static constexpr ahci_reg ccc_ctl		{ 0x14 };
// 	static constexpr ahci_reg ccc_ports		{ 0x18 };
// 	static constexpr ahci_reg em_loc		{ 0x1c };
// 	static constexpr ahci_reg em_ctl		{ 0x20 };
// 	static constexpr ahci_reg cap2			{ 0x24 };
// 	static constexpr ahci_reg bohc			{ 0x28 };

// 	static constexpr ahci_reg px_cmd				{ 0x100 + 0x00 };
// 	static constexpr ahci_reg cmd_list_basel		{ 0x100 + 0x04 };
// 	static constexpr ahci_reg cmd_list_baseh		{ 0x100 + 0x08 };
// 	static constexpr ahci_reg fis_list_basel		{ 0x100 + 0x0c };
// 	static constexpr ahci_reg fis_list_baseh		{ 0x100 + 0x10 };
// 	static constexpr ahci_reg intr_sts				{ 0x100 + 0x14 };
// 	static constexpr ahci_reg intr_enable			{ 0x100 + 0x18 };
// 	static constexpr ahci_reg cmd_sts				{ 0x100 + 0x1c };
// 	static constexpr ahci_reg taskfile				{ 0x100 + 0x20 };
// 	static constexpr ahci_reg sign					{ 0x100 + 0x24 };
// 	static constexpr ahci_reg sata_sts				{ 0x100 + 0x28 };
// 	static constexpr ahci_reg sata_ctl				{ 0x100 + 0x2c };
// 	static constexpr ahci_reg sata_err				{ 0x100 + 0x30 };
// 	static constexpr ahci_reg sata_active			{ 0x100 + 0x34 };
// 	static constexpr ahci_reg cmd_issue				{ 0x100 + 0x38 };
// 	static constexpr ahci_reg sata_notif			{ 0x100 + 0x3c };
// 	static constexpr ahci_reg fis_based_switch_ctl	{ 0x100 + 0x40 };
// 	static constexpr ahci_reg dev_sleep				{ 0x100 + 0x44 };
// };

punion ahci_cap {
	constexpr ahci_cap(u32 r): raw(r) {}
	pstruct {
		u32 num_ports : 5;
		u32 ext_sata : 1;
		u32 enclosure_mgmt : 1;
		u32 cccs : 1;
		u32 num_cmd_slots : 5;
		u32 psc : 1;
		u32 ssc : 1;
		u32 pmd : 1;
		u32 fbss : 1;
		u32 port_multiplier : 1;
		u32 ahci_only : 1;
		u32 : 1;
		u32 max_speed : 4;
		u32 cmd_list_override : 1;
		u32 led : 1;
		u32 salp : 1;
		u32 sss : 1;
		u32 smps : 1;
		u32 ssntf : 1;
		u32 sncq : 1;
		u32 s64bit : 1;
	};
	u32 raw;
};

punion ahci_ghc {
	constexpr ahci_ghc(u32 r): raw(r) {}
	pstruct {
		u32 reset : 1;
		u32 intr : 1;
		u32 mrsm : 1;
		u32 : 28;
		u32 ahci_enable : 1;
	};
	u32 raw;
};

punion ahci_px_cmd {
	constexpr ahci_px_cmd(u32 r): raw(r) {}
	pstruct {
		u32 start : 1;
		u32 spinup : 1;
		u32 poweron : 1;
		u32 clo : 1;
		u32 fis_recv_enable : 1;
		u32 : 3;
		u32 current_cmd_slot : 5;
		u32 mpss : 1;
		u32 fis_recv_running : 1;
		u32 cmd_list_running : 1;
		u32 cold_presence : 1;
		u32 pm : 1;
		u32 hotplug : 1;
		u32 mpsp : 1;
		u32 cold_presence_support : 1;
		u32 ext_sata : 1;
		u32 fis_based_switching : 1;
		u32 auto_partial_to_slumber : 1;
		u32 atapi : 1;
		u32 led_on_atapi_en : 1;
		u32 : 6;
	};
	u32 raw;
};

punion ahci_sata_ctl {
	constexpr ahci_sata_ctl(u32 r): raw(r) {}
	pstruct {
		u32 det : 4;
		u32 maxspeed : 4;
		u32 pwr_transitions_allowed : 4;
		u32 : 20;
	};
	u32 raw;
};

punion ahci_px_tfd {
	constexpr ahci_px_tfd(u32 r): raw(r) {}
	pstruct {
		pstruct {
			u8 err : 1;
			u8 turi2 : 2;
			u8 data_transfer_req : 1;
			u8 turi : 3;
			u8 busy : 1;
		} sts;
		u32 err : 8;
		u32 : 16;
	};
	u32 raw;
};

pstruct hba_regs {
	u32 caps;
	u32 ghc;
	u32 int_sts;
	u32 port_impl;
	u32 version;
	u32 ccc_ctl; // Command Completion Coalescing Control
	u32 ccc_ports; // Command Completion Coalescing Ports
	u32 em_loc;
	u32 em_ctl;
	u32 cap2;
	u32 bohc; // BIOS/OS Handoff Control and Status

	u8 pad[212];

	pstruct hba_ports {
		u64 cmd_list_base;
		u64 fis_list_base;
		u32 intr_sts;
		u32 intr_enable;
		u32 cmd_sts;
		u32 res;
		u32 taskfile;
		u32 sign;
		u32 sata_sts;
		u32 sata_ctl;
		u32 sata_err;
		u32 sata_active;
		u32 cmd_issue;
		u32 sata_notif;
		u32 fis_based_switch_ctl;
		u32 dev_sleep;

		u8 pad[56];
	} ports[];
};

pstruct prdt_entry {
	u64 data_base;
	
	u32 : 32;
	
	u32 bytes : 22;
	u32 : 9;
	u32 ioc : 1;
};

pstruct cmd_table {
	u8 cmd_fis[64];
	u8 atapi_cmd[16];
	u8 reserved[48];

	prdt_entry entries[];
};

pstruct cmd_hdr {
	u32 cmd_fis_len : 5;
	u32 atapi : 1;
	u32 write : 1;
	u32 prefetchable : 1;
	u32 reset : 1;
	u32 bist : 1; // Built-In Self Test
	u32 clear_buzi_upon_ok : 1;
	u32 : 1;
	u32 pm_port : 4;
	u32 prdtlen : 16;

	u32 bytes;

	u64 cmd_table_base;

	u32 reserved[4];
};

struct FisTypes {
	static constexpr u8 REG_H2D	= 0x27;	// Register FIS - host to device
	static constexpr u8 REG_D2H	= 0x34;	// Register FIS - device to host
	static constexpr u8 DMA_ACT	= 0x39;	// DMA activate FIS - device to host
	static constexpr u8 DMA_SETUP	= 0x41;	// DMA setup FIS - bidirectional
	static constexpr u8 DATA		= 0x46;	// Data FIS - bidirectional
	static constexpr u8 BIST		= 0x58;	// BIST activate FIS - bidirectional
	static constexpr u8 PIO_SETUP	= 0x5F;	// PIO setup FIS - device to host
	static constexpr u8 DEV_BITS	= 0xA1;	// Set device bits FIS - device to host
};

pstruct fis_reg_h2d {
	u8 type;	// FIS_TYPE_REG_H2D
	u8 pmport : 4;
	u8 : 3;
	u8 cmd_ctl : 1;
	u8 cmd;
	u8 featurel;
	
	u8 lba0;
	u8 lba1;
	u8 lba2;
	u8 device;

	u8 lba3;
	u8 lba4;
	u8 lba5;
	u8 featureh;

	// DWORD 3
	// u8 countl;		// Count register, 7:0
	// u8 counth;		// Count register, 15:8
	u16 count;
	u8 icc; // isochronous command control
	u8 control;
	
	u8 reserved[4];
};

pstruct fis_reg_d2h {
	u8 type;	// FIS_TYPE_REG_H2D
	u8 pmport : 4;
	u8 : 2;
	u8 intr : 1;
	u8 : 1;
	u8 sts;
	u8 err;
	
	u8 lba0;
	u8 lba1;
	u8 lba2;
	u8 device;

	u8 lba3;
	u8 lba4;
	u8 lba5;
	u8 : 8;

	// u8 countl;		// Count register, 7:0
	// u8 counth;		// Count register, 15:8
	u16 count;
	u8 reserved1[2];

	u8 reserved2[4];
};

enum {
	PxCMD_ST = 0,
	PxCMD_FRE = 4,
	PxCMD_FR = 14,
	PxCMD_CR = 15,

	PxTFD_BSY = 7,
	PxTFD_DRQ = 3,
	PxTFD_ERR = 0,
};
