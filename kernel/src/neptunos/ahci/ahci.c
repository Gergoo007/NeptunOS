#include "ahci.h"
#include <neptunos/graphics/text_renderer.h>

hba_mem_t* abar = NULL;
ahci_port_t* ports[32];
uint8_t port_i = 0;

void ahci_configure_port(ahci_port_t* port) {
	stop_cmd(port);

	void* cmd_base = request_page();
	port->hba_port->cmd_list_base = (uint32_t)((uint64_t)cmd_base);
	port->hba_port->cmd_list_base_upper = (uint32_t)((uint64_t)cmd_base >> 32);
	// TODO: fix this
	//memset(port->hba_port->cmd_list_base, 0, 1024);
	memset(cmd_base, 0, 1024);

	void* fis_base = request_page();
	port->hba_port->fis_base_addr = (uint32_t)(uint64_t)fis_base;
	port->hba_port->cmd_list_base_upper = (uint32_t)((uint64_t)fis_base >> 32);
	memset(fis_base, 0, 256);

	hba_cmd_hdr_t* cmd_hdr = (hba_cmd_hdr_t*)((uint64_t)port->hba_port->cmd_list_base + ((uint64_t)port->hba_port->cmd_list_base_upper << 32));
	
	for (uint8_t i = 0; i < 32; i++) {
		cmd_hdr[i].prdt_length = 8;

		void* cmd_table_addr = request_page();
		uint64_t addr = (uint64_t) cmd_table_addr + (i << 8);
		cmd_hdr[i].cmd_table_base_addr = (uint32_t)((uint64_t)addr);
		cmd_hdr[i].cmd_table_base_addr_upper = (uint32_t)((uint64_t)addr >> 32);
		memset(cmd_table_addr, 0, 256);
	}

	start_cmd(port);
}

void stop_cmd(ahci_port_t* port) {
	// ahci_port_cmd_status_t* status = (ahci_port_cmd_status_t*)&port->hba_port->cmd_status;
	// status->start = 0;
	// status->fis_rec_enable = 0;
	
	port->hba_port->cmd_status &= ~HBA_PxCMD_ST;
	port->hba_port->cmd_status &= ~HBA_PxCMD_FRE;

	// Wait until command finish
	while (1) {
		// if (status->fis_rec_running) continue;
		// if (status->cmd_list_running) continue;
		if (port->hba_port->cmd_status & HBA_PxCMD_FR) continue;
		if (port->hba_port->cmd_status & HBA_PxCMD_CR) continue;

		break;
	}
}

void start_cmd(ahci_port_t* port) {
	// ahci_port_cmd_status_t* status = (ahci_port_cmd_status_t*)&port->hba_port->cmd_status;
	// while (status->cmd_list_running);

	// status->fis_rec_enable = 1;
	// status->start = 1;
	while (port->hba_port->cmd_status & HBA_PxCMD_CR);

	port->hba_port->cmd_status |= HBA_PxCMD_FRE;
	port->hba_port->cmd_status |= HBA_PxCMD_ST;
}

ahci_port_type ahci_check_port_type(hba_port_t* port) {
	sata_status_t status = *((sata_status_t*) &port->sata_status);

	// Device cannot be communicated with
	if (status.det != DET_PRESENT_PHY) return AHCI_NONE;
	// Device not powered on
	if (status.ipm != IPM_ACTIVE) return AHCI_NONE;
	// Device cannot be communicated with
	if (status.spd == SPD_NOT_DETECTED) return AHCI_NONE;

	switch (port->signature) {
		case SATA_SIG_ATAPI:
			return AHCI_SATAPI;

		case SATA_SIG_ATA:
			return AHCI_SATA;

		case SATA_SIG_PM:
			return AHCI_PM;

		case SATA_SIG_SEMB:
			return AHCI_SEMB;

		case SATA_SIG_NONE:
			return AHCI_NONE;

		default:
			printk("\nUnknown signature: %04x\n", port->signature);
			return AHCI_NONE;
	}
}

void probe_ports() {
	assert(!abar, "Error: abar is NULL");

	uint32_t ports_impl = abar->ports_impl;
	for (uint8_t i = 0; i < 32; i++) {
		if (ports_impl & (1 << i)) {
			ahci_port_type port_type = ahci_check_port_type(&abar->ports[i]);

			if (port_type == AHCI_SATA || port_type == AHCI_SATAPI) {
				ports[port_i] = malloc(sizeof(ahci_port_t));
				reportln("Amount allocated: %04x", sizeof(ahci_port_t));
				ports[port_i]->type = port_type;
				ports[port_i]->hba_port = &(abar->ports[i]);
				ports[port_i]->num = port_i;
				port_i++;
			}
		}
	}
}

void ahci_init(pci_device_header_0_t* device) {
	//assert(sizeof(ahci_port_cmd_status_t) != 4, "ahci_port_cmd_status_t is of wrong size!");
	reportln("Device to be used for AHCI: %04x:%04x > %s %s", device->base.vendor_id, device->base.device_id, 
		pci_find_class(device->base.class), 
		pci_find_subclass(device->base.class, device->base.subclass));

	abar = (hba_mem_t*)((uint64_t)device->base_addr_5);
	map_address(abar, abar);

	probe_ports();

	for (uint8_t i = 0; i < port_i; i++) {
		ahci_port_t* port = ports[i];
		ahci_configure_port(port);

		// Read
		port->buffer = request_page();
		memset(port->buffer, 0, 0x1000);

		reportln("Now reading...");
		uint8_t ret = ahci_read(port, 0, 4, port->buffer);
		reportln("Result of read: %d", ret);

		for (uint64_t j = 0; j < 1024; j++) {
			printk("%c", ((char*)port->buffer)[j]);
		}
	}
}

uint8_t ahci_read(ahci_port_t* port, uint64_t sector, uint16_t sector_count, void* buffer) {
	uint32_t sector_lower = (uint32_t)sector;
	uint32_t sector_upper = (uint32_t)((uint64_t)sector >> 32);

	port->hba_port->int_status = (uint32_t)-1;

	hba_cmd_hdr_t* cmd_hdr = (hba_cmd_hdr_t*) ((uint64_t)port->hba_port->cmd_list_base);
	cmd_hdr->cmd_fis_length = sizeof(fis_hdr_h2d_t) / sizeof(uint32_t);
	cmd_hdr->write = 0;
	cmd_hdr->prdt_length = 1;

	hba_cmd_table_t* cmd_table = (hba_cmd_table_t*)((uint64_t)cmd_hdr->cmd_table_base_addr);
	memset(cmd_table, 0, sizeof(hba_cmd_table_t) + (cmd_hdr->prdt_length-1) * sizeof(hba_prdt_entry_t));

	cmd_table->entries[0].db_addr = (uint32_t) ((uint64_t) buffer);
	cmd_table->entries[0].db_addr_upper = (uint32_t) ((uint64_t) buffer >> 32);
	reportln("Addresses: %08x %08x made from %p", cmd_table->entries[0].db_addr, cmd_table->entries[0].db_addr_upper, buffer);
	cmd_table->entries[0].byte_count = (sector_count << 9) - 1;
	reportln("Bytes to be read: %d\n", cmd_table->entries[0].byte_count);
	cmd_table->entries[0].int_on_completion = 0;

	fis_hdr_h2d_t* cmd_fis = (fis_hdr_h2d_t*) (&cmd_table->cmd_fis);
	cmd_fis->fis_type = FIS_TYPE_REG_H2D;
	cmd_fis->cmd_control = 1;
	cmd_fis->cmd = CMD_ATA_READ_DMA_EX;

	// cmd_fis->lba0 = (uint8_t) sector_lower;
	// cmd_fis->lba1 = (uint8_t) (sector_lower >> 8);
	// cmd_fis->lba2 = (uint8_t) (sector_lower >> 16);
	// cmd_fis->lba3 = (uint8_t) sector_upper;
	// cmd_fis->lba4 = (uint8_t) (sector_upper >> 8);
	// cmd_fis->lba4 = (uint8_t) (sector_upper >> 16);

	cmd_fis->lba0 = (uint8_t) sector_lower;
	cmd_fis->lba1 = (uint8_t) (sector_lower >> 8);
	cmd_fis->lba2 = (uint8_t) (sector_lower >> 16);
	cmd_fis->lba3 = (uint8_t) (sector_lower >> 24);
	cmd_fis->lba4 = (uint8_t) (sector_upper);
	cmd_fis->lba4 = (uint8_t) (sector_upper >> 8);
	
	cmd_fis->device_register = 1 << 6; // LBA mode

	cmd_fis->count_low = sector_count & 0xff;
	cmd_fis->count_high = (sector_count >> 8) & 0xff;

	report("Counts (l & h): %02x %02x", cmd_fis->count_low, cmd_fis->count_high);

	uint64_t spin = 0;
	while ((port->hba_port->task_file_data & (AHCI_ATA_DEVICE_BUSY | AHCI_ATA_DEVICE_DRQ)) && spin < 1000000) {
		reportln("hey");
		spin++;
	}
	assert(spin == 1000000, "Device is stuck!");

	reportln("Spin: %d\n", spin);

	// Enter command
	port->hba_port->cmd_issue = 1;
	reportln("Command issue %d\n", port->hba_port->cmd_issue);

	reportln("int_status: %32b", port->hba_port->int_status);

	// Wait until it completes
	while (1) {
		if (port->hba_port->int_status & HBA_PORT_IS_TFES) {
			panic("Read unsuccessful!");
			return 1;
		}
		if (port->hba_port->cmd_issue == 0) {
			reportln("Finished!");
			break;
		}
	}

	return 0;
}
