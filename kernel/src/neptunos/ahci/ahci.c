#include "ahci.h"

// Start command engine
void start_cmd(hba_port_t* port) {
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PxCMD_CR);

	// Set FRE (bit4) and ST (bit0)
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST;
}
 
// Stop command engine
void stop_cmd(hba_port_t* port) {
	// Clear ST (bit0)
	port->cmd &= ~HBA_PxCMD_ST;
 
	// Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;
 
	// Wait until FR (bit14), CR (bit15) are cleared
	while(1) {
		if (port->cmd & HBA_PxCMD_FR)
			continue;
		if (port->cmd & HBA_PxCMD_CR)
			continue;
		break;
	}
}

void port_rebase(hba_port_t* port) {
	stop_cmd(port);

	// Command list entry size = 32
	// Command list entry maxim count = 32
	// Command list maxim size = 32*32 = 1K per port
	void* cmd_list_base = request_page();
	port->clb = (u32)(u64)cmd_list_base;
	port->clbu = (u32)((u64)cmd_list_base >> 32);
	memset(cmd_list_base, 0, 1024);

	// FIS entry size = 256 bytes per port
	void* fis_base = request_page();
	port->fb = (u32)(u64)fis_base;
	port->fbu = (u32)((u64)fis_base >> 32);
	memset(fis_base, 0, 256);

	// Command table size = 256*32 = 8K per port
	hba_cmd_hdr_t* cmdheader = (hba_cmd_hdr_t*)(u64)(port->clb);
	for (int i = 0; i < 32; i++) {
		cmdheader[i].prdtl = 8;

		void* cmd_table_desc = request_page();
		cmdheader[i].cmd_table_desc_base = (u32)(u64)cmd_table_desc;
		cmdheader[i].cmd_table_desc_base_upper = (u32)((u64)cmd_table_desc >> 32);

		memset(cmd_table_desc, 0, 256);
	}

	start_cmd(port);
}

void ahci_init(pci_device_header_0_t* device) {
	reportln("Initializing AHCI...");
	
	void* abar = (void*)(u64)device->base_addr_5;
	map_page(abar, abar, MAP_FLAGS_DEFAULTS);
	
	hba_mem_t* hba = (hba_mem_t*)abar;
	
	for (u8 i = 0; i < 32; i++) {
		// Port is implemented
		if ((hba->pi >> i) & 1) {
			if (hba->ports[i].ssts.det == AHCI_DEV_DET_PRESENT &&
			hba->ports[i].ssts.ipm == AHCI_DEV_IPM_ACTIVE &&
			hba->ports[i].ssts.spd != AHCI_DEV_SPD_NO_DEV) {
				printk("Port %d available!\n", i);
				printk("Signature: ");
				// Rebase port
				port_rebase(&hba->ports[i]);
				switch (hba->ports[i].sig) {
					case AHCI_DEV_TYPE_SATAPI:
						printk("SATAPI\n");
						break;
					case AHCI_DEV_TYPE_SEMB:
						printk("SEMB\n");
						break;
					case AHCI_DEV_TYPE_PM:
						printk("PM\n");
						break;
					case AHCI_DEV_TYPE_ATA: {
						void* buf = request_page();
						memset(buf, 0, 0x1000);
						printk("ATA\nAttempting read...\n");
						printk("Return value: %d\n", ahci_read(&hba->ports[i], 0, 1, buf));
						printk("String: %13s\n", buf);
						break;
					}
				}
			}
		}
	}
}

// Find a free command list slot
u8 find_cmdslot(hba_port_t* port) {
	// If not set in SACT and CI, the slot is free
	u32 slots = (port->sact | port->ci);
	for (u8 i = 0; i < 32; i++) {
		if ((slots & 1) == 0)
			return i;

		slots >>= 1;
	}
	error("Cannot find free command list entry\n");
	return 255;
}

u8 ahci_read(hba_port_t* port, u64 start, u32 count, void* buf) {
	port->is = (u32) -1;		// Clear pending interrupt bits
	u8 slot = find_cmdslot(port);
	if (slot == 255)
		return 0;

	hba_cmd_hdr_t* cmdheader = (hba_cmd_hdr_t*)(u64)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(fis_reg_h2d_t)/sizeof(u32);
	cmdheader->write = 0;		// Read from device
	cmdheader->prdtl = (u16)((count-1)>>4) + 1;	// PRDT entries count

	hba_cmd_table_t* cmd_table =
		(hba_cmd_table_t*)((u64)cmdheader->cmd_table_desc_base | ((u64)cmdheader->cmd_table_desc_base_upper << 32));
	memset(cmd_table, 0, sizeof(hba_cmd_table_t) + (cmdheader->prdtl-1)*sizeof(hba_cmd_table_t));

	// 8K bytes (16 sectors) per PRDT
	for (u16 i = 0; i < cmdheader->prdtl-1; i++) {
		cmd_table->prdt_entry[i].dba = (u32)(u64)buf;
		cmd_table->prdt_entry[i].dbc = 8*1024-1;	// 8K bytes (this value should always be set to 1 less than the actual value)
		cmd_table->prdt_entry[i].int_ = 1;
		buf += 4*1024;	// 4K words
		count -= 16;	// 16 sectors
	}
	// Last entry
	cmd_table->prdt_entry[cmdheader->prdtl-1].dba = (u32)(u64)buf;
	cmd_table->prdt_entry[cmdheader->prdtl-1].dbc = (count<<9)-1;	// 512 bytes per sector
	cmd_table->prdt_entry[cmdheader->prdtl-1].int_ = 1;

	// Setup command
	fis_reg_h2d_t* cmdfis = (fis_reg_h2d_t*)(&cmd_table->cmd_fis);

	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->cmd_ctl = 1;	// Command
	cmdfis->cmd = ATA_CMD_READ_DMA_EX;
	cmdfis->device = 1<<6;	// LBA mode

	cmdfis->lba0 = (u8)start;
	cmdfis->lba1 = (u8)(start>>8);
	cmdfis->lba2 = (u8)(start>>16);

	cmdfis->lba3 = (u8)(start>>24);
	cmdfis->lba4 = (u8)(start>>32);
	cmdfis->lba5 = (u8)(start>>48);

	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;

	// The below loop waits until the port is no longer busy before issuing a new command
	u32 spin = 0;
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
		spin++;

	if (spin == 1000000) {
		error("Port is hung\n");
		return 0;
	}

	port->ci = 1<<slot;	// Issue command

	// Wait for completion
	while (1) {
		// In some longer duration reads, it may be helpful to spin on the DPS bit
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1<<slot)) == 0)
			break;

		if (port->is & HBA_PxIS_TFES) {	// Task file error
			error("Read disk error\n");
			return 0;
		}
	}

	// Check again
	if (port->is & HBA_PxIS_TFES) {
		error("Read disk error\n");
		return 0;
	}

	return 1;
}
