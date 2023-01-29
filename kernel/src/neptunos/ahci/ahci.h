#pragma once

#include <neptunos/libk/stdall.h>
#include <neptunos/pci/pci.h> 

#define CMD_ATA_READ_DMA_EX 0x25

#define AHCI_ATA_DEVICE_BUSY 0x80
#define AHCI_ATA_DEVICE_DRQ 0x08

#define HBA_PORT_IS_TFES (1 << 30)

#define HBA_PxCMD_CR 0x8000
#define HBA_PxCMD_FRE 0x0010
#define HBA_PxCMD_ST 0x0001
#define HBA_PxCMD_FR 0x4000

// Host bus adapter port
typedef struct hba_port_t {
	uint32_t cmd_list_base;
	uint32_t cmd_list_base_upper;
	uint32_t fis_base_addr;
	uint32_t fis_base_addr_upper;
	uint32_t int_status;
	uint32_t int_enable;
	uint32_t cmd_status;
	uint32_t reserved1;
	uint32_t task_file_data;
	uint32_t signature;
	uint32_t sata_status;
	uint32_t sata_ctl;
	uint32_t sata_error;
	uint32_t sata_active;
	uint32_t cmd_issue;
	uint32_t sata_notif;
	uint32_t fis_switch_ctl;
	uint32_t reserved2[11];
	uint32_t vendor_data[4];
} hba_port_t;

typedef struct hba_mem_t {
	uint32_t host_capability;
	uint32_t global_host_ctl;
	uint32_t int_status;
	uint32_t ports_impl;
	uint32_t version;
	uint32_t ccc_ctl;
	uint32_t ccc_ports;
	uint32_t enclosure_mgr_location;
	uint32_t enclosure_mgr_ctl;
	uint32_t host_capabilities_ext;
	uint32_t bios_handoff_ctl_status;
	uint8_t reserved[0x74];
	uint8_t vendor[0x60];
	hba_port_t ports[1];
} _attr_packed hba_mem_t;

typedef struct sata_status_t {
	uint8_t det : 4;
	uint8_t spd : 4;
	uint8_t ipm : 4;
	uint32_t reserved : 22;
} _attr_packed sata_status_t;

typedef enum ahci_port_type {
	AHCI_NONE = 0,
	AHCI_SATA = 1,
	AHCI_SEMB = 2,
	AHCI_PM = 3,
	AHCI_SATAPI = 4,
} ahci_port_type;

typedef enum SATA_STATUSES {
	DET_NOT_DETECTED = 0x00,
	DET_PRESENT_NO_PHY = 0x01,
	DET_PRESENT_PHY = 0x03,
	DET_DISABLED = 0x04,
} SATA_STATUSES;

typedef enum IPM_STATUSES {
	IPM_NOT_DETECTED = 0x00,
	IPM_ACTIVE = 0x01,
	IPM_PARTIAL_PWR = 0x02,
	IPM_SLUMBER_PWR = 0x06,
} IPM_STATUSES;

typedef enum SPD_STATUSES {
	SPD_NOT_DETECTED = 0x00,
	SPD_GEN_1 = 0x01,
	SPD_GEN_2 = 0x02,
	SPD_GEN_3 = 0x03,
} SPD_STATUSES;

typedef enum SATA_SIGNATURES {
	SATA_SIG_ATAPI = 0xeb140101,
	SATA_SIG_ATA = 0x00000101,
	SATA_SIG_SEMB = 0xc33c0101,
	SATA_SIG_PM	= 0x96690101,
	SATA_SIG_NONE = 0xffff0101,
} SATA_SIGNATURES;

typedef struct ahci_port_t {
	hba_port_t* hba_port;
	ahci_port_type type;
	void* buffer;
	uint8_t num;
} ahci_port_t;

typedef enum {
	HBA_CMD_STATUS_CR = 0x8000,
	HBA_CMD_STATUS_FRE = 0x0010,
	HBA_CMD_STATUS_ST = 0x0001,
} AHCI_PORT_CMD_STATUS;

// typedef struct ahci_port_cmd_status_t {
// 	uint8_t start : 1;
// 	uint8_t spin_up : 1;
// 	uint8_t power_on : 1;
// 	uint8_t cmd_list_override : 1;
// 	uint8_t fis_rec_enable : 1;
// 	uint16_t reserved : 9;
// 	uint8_t fis_rec_running : 1;
// 	uint8_t cmd_list_running : 1;
// 	uint16_t reserved2 : 16;
// } _attr_packed ahci_port_cmd_status_t;

typedef struct hba_cmd_hdr_t {
	uint8_t cmd_fis_length : 5;
	uint8_t atapi : 1;
	uint8_t write : 1;
	uint8_t prefetchable : 1;
	uint8_t reset : 1;
	uint8_t bist : 1;
	uint8_t clear_busy : 1;
	uint8_t reserved1 : 1;
	uint8_t pm : 4;
	uint16_t prdt_length;
	uint16_t prdb_count;
	uint32_t cmd_table_base_addr;
	uint32_t cmd_table_base_addr_upper;
	uint32_t res2[4];
} _attr_packed hba_cmd_hdr_t;

typedef struct fis_hdr_h2d_t {
	uint8_t fis_type;
	uint8_t pm : 4;
	uint8_t reserved1 : 3;
	uint8_t cmd_control : 1;
	uint8_t cmd;
	uint8_t feature;
	uint8_t lba0;
	uint8_t lba1;
	uint8_t lba2;
	uint8_t device_register;
	uint8_t lba3;
	uint8_t lba4;
	uint8_t lba5;
	uint8_t feature_high;
	uint8_t count_low;
	uint8_t count_high;
	uint8_t iso_cmd_completion;
	uint8_t ctl;
	uint8_t reserved2[4];
} _attr_packed fis_hdr_h2d_t;

typedef struct hba_prdt_entry_t {
	uint32_t db_addr;
	uint32_t db_addr_upper;
	uint32_t reserved1;
	uint32_t byte_count : 22;
	uint32_t reserved2 : 9;
	uint32_t int_on_completion : 1;
} _attr_packed hba_prdt_entry_t;

typedef struct hba_cmd_table_t {
	uint8_t cmd_fis[64];
	uint8_t atapi_cmd[16];
	uint8_t reserved[48];
	hba_prdt_entry_t entries[];
} _attr_packed hba_cmd_table_t;

enum {
	FIS_TYPE_REG_H2D = 0x27,
	FIS_TYPE_REG_D2H = 0x34,
	FIS_TYPE_DMA_ACT = 0x39,
	FIS_TYPE_DMA_SETUP = 0x41,
	FIS_TYPE_DATA = 0x46,
	FIS_TYPE_BIST = 0x58,
	FIS_TYPE_PIO_SETUP = 0x5f,
	FIS_TYPE_DEVICE_BITS = 0xa1,
};

extern hba_mem_t* abar;
extern ahci_port_t* ports[32];
extern uint8_t port_i;

ahci_port_type ahci_check_port_type(hba_port_t* port);

void ahci_init(pci_device_header_0_t* device);
void ahci_configure_port(ahci_port_t* port);

void stop_cmd(ahci_port_t* port);
void start_cmd(ahci_port_t* port);

uint8_t ahci_read(ahci_port_t* port, uint64_t sector, uint16_t sector_count, void* buffer);
