#pragma once

#include <neptunos/libk/stdall.h>
#include <neptunos/pci/pci.h>

// Host bus adapter port
typedef struct hba_port {
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
} hba_port;

typedef struct hba_mem {
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
	hba_port ports[1];
} _attr_packed hba_mem;

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

extern hba_mem* abar;

ahci_port_type ahci_check_port_type(hba_port* port);

void ahci_init(pci_device_header_0_t* device);
