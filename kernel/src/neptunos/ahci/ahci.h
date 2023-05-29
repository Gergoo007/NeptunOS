#pragma once

#include <neptunos/pci/pci.h>

typedef enum {
	AHCI_DEV_TYPE_ATA = 0x00000101,
	AHCI_DEV_TYPE_SATAPI = 0xEB140101,
	AHCI_DEV_TYPE_SEMB = 0xC33C0101,
	AHCI_DEV_TYPE_PM = 0x96690101,
} AHCI_DEV_TYPES;

typedef enum {
	AHCI_DEV_DET_NO_DEV = 0,
	AHCI_DEV_DET_NO_PHYS = 1,
	AHCI_DEV_DET_PRESENT = 3,
	AHCI_DEV_DET_DISABLED = 4,
} AHCI_DEV_DET;

typedef enum {
	AHCI_DEV_SPD_NO_DEV = 0,
	AHCI_DEV_SPD_GEN_1 = 1,
	AHCI_DEV_SPD_GEN_2 = 2,
} AHCI_DEV_SPD;

typedef enum {
	AHCI_DEV_IPM_NO_DEV		= 0,
	AHCI_DEV_IPM_ACTIVE		= 1,
	AHCI_DEV_IPM_PARTIAL	= 2,
	AHCI_DEV_IPM_SLUMBER	= 6,
} AHCI_DEV_IPM;

typedef enum {
	HBA_PxCMD_ST	= 0x0001,
	HBA_PxCMD_FRE	= 0x0010,
	HBA_PxCMD_FR	= 0x4000,
	HBA_PxCMD_CR	= 0x8000,
} AHCI_PORT_CMD;

typedef enum {
	FIS_TYPE_REG_H2D	= 0x27,
	FIS_TYPE_REG_D2H	= 0x34,
	FIS_TYPE_DMA_ACT	= 0x39,
	FIS_TYPE_DMA_SETUP	= 0x41,
	FIS_TYPE_DATA		= 0x46,
	FIS_TYPE_BIST		= 0x58,
	FIS_TYPE_PIO_SETUP	= 0x5F,
	FIS_TYPE_DEV_BITS	= 0xA1,
} AHCI_FIS_TYPE;

typedef enum {
	ATA_DEV_BUSY = 0x80,
	ATA_DEV_DRQ = 0x08,
} AHCI_ATA;

typedef enum {
	ATA_CMD_READ_DMA		= 0xC8,
	ATA_CMD_READ_DMA_EX		= 0x25,
	ATA_CMD_WRITE_DMA		= 0xCA,
	ATA_CMD_WRITE_DMA_EX	= 0x35,
} AHCI_ATA_CMD;

typedef enum {
	HBA_PxIS_TFES = 1 << 30,
} AHCI_PORT_INT_STATUS;

typedef struct fis_reg_h2d {
	u8 fis_type;	// FIS_TYPE_REG_H2D

	u8 pmport : 4;	// Port multiplier
	u8 : 3;
	u8 cmd_ctl : 1;	// 1: Command, 0: Control

	u8 cmd;			// Command register
	u8 featurel;	// Feature register, 7:0

	u8 lba0;		// LBA low register, 7:0
	u8 lba1;		// LBA mid register, 15:8
	u8 lba2;		// LBA high register, 23:16
	u8 device;		// Device register

	u8 lba3;		// LBA register, 31:24
	u8 lba4;		// LBA register, 39:32
	u8 lba5;		// LBA register, 47:40
	u8 featureh;	// Feature register, 15:8

	u8 countl;		// Count register, 7:0
	u8 counth;		// Count register, 15:8
	u8 icc;			// Isochronous command completion
	u8 ctl;			// Control register

	u8 rsv1[4];		// Reserved
} _attr_packed fis_reg_h2d_t;

typedef struct hba_prdt_entry {
	u32 dba;		// Data base address
	u32 dbau;		// Data base address upper 32 bits
	u32 rsv0;		// Reserved

	// DW3
	u32 dbc : 22;	// Byte count, 4M max
	u32 : 9;		// Reserved
	u32 int_ : 1;	// Interrupt on completion
} _attr_packed hba_prdt_entry_t;

typedef struct hba_cmd_table {
	// 0x00
	u8 cmd_fis[64];	// Command FIS

	// 0x40
	u8 atapi_cmd[16];	// ATAPI command, 12 or 16 bytes

	// 0x50
	u8 rsv[48];	// Reserved

	// 0x80
	hba_prdt_entry_t prdt_entry[1];	// Physical region descriptor table entries, 0 ~ 65535
} _attr_packed hba_cmd_table_t;

typedef struct hba_port {
	u32 clb;		// 0x00, command list base address, 1K-byte aligned
	u32 clbu;		// 0x04, command list base address upper 32 bits
	u32 fb;			// 0x08, FIS base address, 256-byte aligned
	u32 fbu;		// 0x0C, FIS base address upper 32 bits
	u32 is;			// 0x10, interrupt status
	u32 ie;			// 0x14, interrupt enable
	u32 cmd;		// 0x18, command and status
	u32 rsv0;		// 0x1C, Reserved
	u32 tfd;		// 0x20, task file data
	u32 sig;		// 0x24, signature
	struct {
		u8 det : 4;
		u8 spd : 4;
		u8 ipm : 4;
		u32 : 20;
	} ssts;		// 0x28, SATA status (SCR0:SStatus)
	u32 sctl;		// 0x2C, SATA control (SCR2:SControl)
	u32 serr;		// 0x30, SATA error (SCR1:SError)
	u32 sact;		// 0x34, SATA active (SCR3:SActive)
	u32 ci;			// 0x38, command issue
	u32 sntf;		// 0x3C, SATA notification (SCR4:SNotification)
	u32 fbs;		// 0x40, FIS-based switch control
	u32 rsv1[11];	// 0x44 ~ 0x6F, Reserved
	u32 vendor[4];	// 0x70 ~ 0x7F, vendor specific
} _attr_packed hba_port_t;

typedef struct hba_mem {
	u32 cap;		// 0x00, Host capability
	u32 ghc;		// 0x04, Global host control
	u32 is;			// 0x08, Interrupt status
	u32 pi;			// 0x0C, Port implemented
	u32 vs;			// 0x10, Version
	u32 ccc_ctl;	// 0x14, Command completion coalescing control
	u32 ccc_pts;	// 0x18, Command completion coalescing ports
	u32 em_loc;		// 0x1C, Enclosure management location
	u32 em_ctl;		// 0x20, Enclosure management control
	u32 cap2;		// 0x24, Host capabilities extended
	u32 bohc;		// 0x28, BIOS/OS handoff control and status

	// 0x2C - 0x9F, Reserved
	u8 rsv[0xA0-0x2C];

	// 0xA0 - 0xFF, Vendor specific registers
	u8 vendor[0x100-0xA0];

	// 0x100 - 0x10FF, Port control registers
	hba_port_t	ports[32];	// 1 ~ 32
} _attr_packed hba_mem_t;

typedef struct hba_cmd_hdr {
	// DW0
	u8 cfl : 5;		// Command FIS length in DWORDS, 2 ~ 16
	u8 atapi : 1;	// ATAPI
	u8 write : 1;	// Write, 1: H2D, 0: D2H
	u8 prefetch : 1;// Prefetchable

	u8 rst : 1;		// Reset
	u8 bist : 1;	// BIST
	u8 c : 1;		// Clear busy upon R_OK
	u8 : 1;			// Reserved
	u8 pmp : 4;		// Port multiplier port

	u16 prdtl;		// Physical region descriptor table length in entries

	// DW1
	volatile u32 prdbc;	// Physical region descriptor byte count transferred

	// DW2, 3
	u32 cmd_table_desc_base;		// Command table descriptor base address
	u32 cmd_table_desc_base_upper;	// Command table descriptor base address upper 32 bits

	// DW4 - 7
	u32 res[4];
} _attr_packed hba_cmd_hdr_t;

void ahci_init(pci_device_header_0_t *device);
u8 ahci_read(hba_port_t* port, u64 start, u32 count, void* buf);
