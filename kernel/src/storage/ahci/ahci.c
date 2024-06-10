#include <storage/ahci/ahci.h>

ahci* ahcis = 0;
u32 num_ahcis = 0;

hba_port* tmp;

void ahci_stop_cmds(hba_port* port) {
	port->start = 0;
	port->fis_rx_enable = 0;

	// Meg kell várni amíg ezek átváltanak
	while (port->start);
	while (port->fis_rx_enable);
}

void ahci_start_cmds(hba_port* port) {
	port->start = 1;
	port->fis_rx_enable = 1;

	// Meg kell várni amíg ezek átváltanak
	while (!port->start);
	while (!port->fis_rx_enable);
}

void ahci_init(pci_hdr_t* pci) {
	ahcis = request_page();

	// TODO: caching
	hba_mem* regs = (hba_mem*)(u64)pci->type0.bar5;

	for (u32 port = 0; port < 32; port++) {
		// Port nincs jelen a kontrollerben
		if (!(regs->ports_impl & (1 << port))) continue;

		// Port státusza (spd: speed; det: csatlakoztatva; ipm: áram)
		if (regs->ports[port].det != 0x03) continue;
		if (regs->ports[port].spd == 0x00) continue;
		if (regs->ports[port].ipm == 0x00) continue;

		switch (regs->ports[port].sig) {
			case AHCI_SIG_SATA: {
				printk("SATA eszkoz!\n");
				tmp = &regs->ports[port];
				break;
			}
			case AHCI_SIG_SATAPI: printk("SATAPI eszkoz!\n"); break;
		}

		// Port előkészítése
		ahci_stop_cmds(&regs->ports[port]);

		// Parancslista beállítása
		// Egy sor 32 byte, 32 sor van tehát 1024 byte a lista
		// A lista után van a FIS
		void* mem = pmm_alloc_page();
		memset(mem, 0, 1024+256);
		regs->ports[port].cmd_listl = (u64)mem & 0xffffffff;
		regs->ports[port].cmd_listu = (u64)mem >> 32;

		hba_cmd_hdr* hdr = mem;

		mem += 1024;

		regs->ports[port].fisl = (u64)mem & 0xffffffff;
		regs->ports[port].fisu = (u64)mem >> 32;

		// Parancslista
		for (u8 i = 0; i < 32; i++) {
			hdr[i].prdt_num_entries = 8;
			
			// Parancs táblázat létrehozása
			// TODO: a command table 8 KiB de itt csak 4 KiB-ot foglalok le
			void* cmd_table = pmm_alloc_page();
			sprintk("begin: %p %p\n\r", &hdr, i);
			hdr[i].cmd_table_desc_l = (u64)cmd_table & 0xffffffff;
			hdr[i].cmd_table_desc_u = (u64)cmd_table >> 32;
			sprintk("end\n\r");

			// memset(cmd_table, 0, 256);
		}

		ahci_start_cmds(&regs->ports[port]);
	}
}

// TODO: BEFEJEZNI!
u32 ahci_find_cmd_slot(hba_port* port) {
	u32 slots = (port->sactive | port->cmd_issue);
	for (u32 i = 0; i < 10; i++) {

	}
}

void ahci_read(hba_port* port, u64 start, u64 count, void* buf) {
	u32 starth = start >> 32;
	u32 startl = start & 0xffffffff;

	port->int_sts = (u32)-1;

	// Egyelőre feltételezem hogy az első slot szabad
	u32 slot = 0;

	hba_cmd_hdr* hdr = (hba_cmd_hdr*)(port->cmd_listl | ((u64)port->cmd_listu << 32));
	hdr += slot;
	hdr->fis_len = sizeof(fis_reg_h2d) / sizeof(u32); // HA NEM MŰXIK: Hozzá kell majd adni egyet
	hdr->write = 0;
	hdr->prdt_num_entries = (u16) ((count - 1) >> 4) + 1;

	hba_cmd_table* table = (hba_cmd_table*)(hdr->cmd_table_desc_l | ((u64)hdr->cmd_table_desc_u << 32));
	memset((void*)table, 0, sizeof(hba_cmd_table) + (hdr->prdt_num_entries-1) * sizeof(hba_prdt_entry));

	// 16 szektor per PRDT
	u32 i = 0;
	for (; i < hdr->prdt_num_entries-1; i++) {
		table->prdt_entry[i].data_base_addrl = (u64)buf & 0xffffffff;
		table->prdt_entry[i].data_base_addru = (u64)buf << 32;

		table->prdt_entry[i].num_bytes = 8 * 1024 - 1;
		table->prdt_entry[i].ioc = 1;

		buf += 0x2000; // 16 szektor * 512 byte
		count -= 16;
	}

	table->prdt_entry[i].data_base_addrl = (u64)buf & 0xffffffff;
	table->prdt_entry[i].data_base_addru = (u64)buf << 32;

	table->prdt_entry[i].num_bytes = (count << 9) - 1;
	table->prdt_entry[i].ioc = 1;

	// Setup parancs
	fis_reg_h2d* cmd_fis = (fis_reg_h2d*)table->cmd_fis;
	cmd_fis->type = AHCI_FIS_REG_H2D;
	cmd_fis->cmd_ctrl = 1;
	cmd_fis->cmd = 0x25; // TODO: enumok ezekről

	cmd_fis->lba0 = (u8) (startl);
	cmd_fis->lba1 = (u8) (startl >> 8);
	cmd_fis->lba2 = (u8) (startl >> 16);
	cmd_fis->dev = 1 << 6; // Ez elvileg LBA mód

	cmd_fis->lba3 = (u8) (startl >> 24);
	cmd_fis->lba4 = (u8) (starth);
	cmd_fis->lba5 = (u8) (starth >> 8);

	cmd_fis->countl = count & 0xff;
	cmd_fis->counth = (count >> 8) & 0xff;

	// Meg kell várni amíg a port szabad lesz
	u64 spin = 0;
	while ((port->task_file_data & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
		spin++;
	}
	if (spin == 1000000) printk("Port elakadt!\n");

	port->cmd_issue = 1 << slot;

	while (1) {
		if ((port->cmd_issue & (1 << slot)) == 0) break;

		if (port->int_sts & (1 << 30)) { // TODO: enum
			printk("Ajajj\n");
		}
	}
}
