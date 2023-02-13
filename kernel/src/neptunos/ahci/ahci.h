#pragma once

#include <neptunos/libk/stdall.h>
#include <neptunos/pci/pci.h> 

typedef struct hba_port_regs_t {
	
} _attr_packed hba_port_regs_t;

typedef struct hba_mem_regs_t {
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
	hba_port_regs_t* ports;
} _attr_packed hba_mem_regs_t;

typedef enum
{
	FIS_TYPE_REG_H2D	= 0x27,	// Register FIS - host to device
	FIS_TYPE_REG_D2H	= 0x34,	// Register FIS - device to host
	FIS_TYPE_DMA_ACT	= 0x39,	// DMA activate FIS - device to host
	FIS_TYPE_DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
	FIS_TYPE_DATA		= 0x46,	// Data FIS - bidirectional
	FIS_TYPE_BIST		= 0x58,	// BIST activate FIS - bidirectional
	FIS_TYPE_PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
	FIS_TYPE_DEV_BITS	= 0xA1,	// Set device bits FIS - device to host
} FIS_t;

extern hba_mem_regs_t* abar;
void ahci_init(pci_device_header_0_t* device);
