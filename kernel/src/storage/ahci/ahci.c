#include <storage/ahci/ahci.h>

ahci* ahcis = 0;
u32 num_ahcis = 0;

void ahci_stop_cmds(hba_port* port) {
	port->cmd.start = 0;
	port->cmd.fis_rx_enable = 0;

	// Meg kell várni amíg ezek átváltanak
	while (port->cmd.start);
	while (port->cmd.fis_rx_enable);
}

void ahci_start_cmds(hba_port* port) {
	port->cmd.start = 1;
	port->cmd.fis_rx_enable = 1;

	// Meg kell várni amíg ezek átváltanak
	while (!port->cmd.start);
	while (!port->cmd.fis_rx_enable);
}

void ahci_init(pci_hdr* pci) {
	ahcis = request_page();

	// Tárhely alrendszer előkészítése
	storage_init();

	hba_mem* regs = (hba_mem*)(u64)pci->type0.bar5;

	for (u32 port = 0; port < 32; port++) {
		// Port nincs jelen a kontrollerben
		if (!(regs->ports_impl & (1 << port))) continue;

		// Port státusza (spd: speed; det: csatlakoztatva; ipm: áram)
		if (regs->ports[port].ssts.det != 0x03) continue;
		if (regs->ports[port].ssts.spd == 0x00) continue;
		if (regs->ports[port].ssts.ipm == 0x00) continue;

		switch (regs->ports[port].sig) {
			case AHCI_SIG_SATA: {
				report("SATA eszkoz!");

				break;
			}
			case AHCI_SIG_SATAPI: report("SATAPI eszkoz!"); break;
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
			hdr[i].cmd_table_desc_l = (u64)cmd_table & 0xffffffff;
			hdr[i].cmd_table_desc_u = (u64)cmd_table >> 32;

			// memset(cmd_table, 0, 256);
		}

		ahci_start_cmds(&regs->ports[port]);
	}

	// ATA portok végigvizsgálása, olvasása
	for (u32 i = 0; i < 32; i++) {
		// Port nincs jelen a kontrollerben
		if (!(regs->ports_impl & (1 << i))) continue;

		// Port státusza (spd: speed; det: csatlakoztatva; ipm: áram)
		if (regs->ports[i].ssts.det != 0x03) continue;
		if (regs->ports[i].ssts.spd == 0x00) continue;
		if (regs->ports[i].ssts.ipm == 0x00) continue;

		if (regs->ports[i].sig != AHCI_SIG_SATA
			&& regs->ports[i].sig != AHCI_SIG_SATAPI) continue;

		// Eszköz hozzáadása a jegyzékhez
		ata_identity* id = ahci_identify(&(regs->ports[i]));
		storage_register(
			regs->ports[i].sig != AHCI_SIG_SATA ? STG_ATAPI : STG_ATA,
			id->sectors_48 * 512,
			id->firmware,
			id->model,
			id->serial,
			(void*)&regs->ports[i]
		);
	}
}

u32 ahci_find_cmd_slot(hba_port* port) {
	u32 slots = (port->sactive | port->cmd_issue);
	for (u32 i = 0; i < 32; i++) {
		if ((slots & 1) == 0) return i;

		slots >>= 1;
	}
	error("Nincs szabad slot!");
	return -1;
}

void ahci_read(hba_port* port, u64 start, u64 count, void* buf) {
	port->int_sts.value = (u32)-1;

	u32 slot = ahci_find_cmd_slot(port);

	hba_cmd_hdr* hdr = (hba_cmd_hdr*)(port->cmd_listl | ((u64)port->cmd_listu << 32));
	hdr += slot;
	hdr->fis_len = sizeof(fis_reg_h2d) / sizeof(u32);
	hdr->write = 0;
	hdr->prdt_num_entries = (u16) ((count - 1) >> 4) + 1;

	hba_cmd_table* table = (hba_cmd_table*)(hdr->cmd_table_desc_l | ((u64)hdr->cmd_table_desc_u << 32));
	memset((void*)table, 0, sizeof(hba_cmd_table) + (hdr->prdt_num_entries-1) * sizeof(hba_prdt_entry));

	// 16 szektor per PRDT
	u32 i = 0;
	for (; i < (u32)hdr->prdt_num_entries-1; i++) {
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
	cmd_fis->cmd = ATA_READ_DMA_EX;

	cmd_fis->lba0 = (u8) (start);
	cmd_fis->lba1 = (u8) (start >> 8);
	cmd_fis->lba2 = (u8) (start >> 16);
	cmd_fis->dev = 1 << 6; // Ez elvileg LBA mód

	cmd_fis->lba3 = (u8) (start >> 24);
	cmd_fis->lba4 = (u8) (start >> 32);
	cmd_fis->lba5 = (u8) (start >> 48);

	cmd_fis->countl = count & 0xff;
	cmd_fis->counth = (count >> 8) & 0xff;

	// Meg kell várni amíg a port szabad lesz
	u64 spin = 0;
	while ((port->task_file_data & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
		spin++;
	}
	if (spin == 1000000) error("Port elakadt!");

	port->cmd_issue = 1 << slot;

	while (1) {
		if ((port->cmd_issue & (1 << slot)) == 0) break;
		if (port->int_sts.task_file_error) { error("ATA read task file error 1"); break; }
	}

	if (port->int_sts.task_file_error) error("ATA read task file error 2");
}

ata_identity* ahci_identify(hba_port* port) {
	ata_identity* id = pmm_alloc_page();
	if (port->sig == AHCI_SIG_SATAPI) {
		memset(id, 0, sizeof(ata_identity));
		return id;
	}

	port->int_sts.value = (u32)-1;

	u32 slot = ahci_find_cmd_slot(port);

	hba_cmd_hdr* hdr = (hba_cmd_hdr*)(port->cmd_listl | ((u64)port->cmd_listu << 32));
	hdr += slot;
	hdr->fis_len = sizeof(fis_reg_h2d) / sizeof(u32);
	hdr->write = 0;
	hdr->prdt_num_entries = 1;

	hba_cmd_table* table = (hba_cmd_table*)(hdr->cmd_table_desc_l | ((u64)hdr->cmd_table_desc_u << 32));
	memset((void*)table, 0, sizeof(hba_cmd_table) + (hdr->prdt_num_entries-1) * sizeof(hba_prdt_entry));

	table->prdt_entry[0].data_base_addrl = (u64)id & 0xffffffff;
	table->prdt_entry[0].data_base_addru = (u64)id << 32;
	table->prdt_entry[0].num_bytes = (1 << 9) - 1;
	table->prdt_entry[0].ioc = 1;

	// Setup parancs
	fis_reg_h2d* cmd_fis = (fis_reg_h2d*)table->cmd_fis;
	memset((void*)cmd_fis, 0, sizeof(fis_reg_h2d));
	cmd_fis->type = AHCI_FIS_REG_H2D;
	cmd_fis->cmd_ctrl = 1;
	cmd_fis->cmd = ATA_CMD_IDENTIFY;
	cmd_fis->dev = 1 << 6;
	cmd_fis->countl = 1;

	// Meg kell várni amíg a port szabad lesz
	u64 spin = 0;
	while ((port->task_file_data & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) spin++;
	if (spin == 1000000) error("Port elakadt!");

	port->cmd_issue = 1 << slot;

	while (1) {
		if ((port->cmd_issue & (1 << slot)) == 0) break;

		if (port->int_sts.task_file_error) {
			error("ATA identify task file error 1\n");
		}
	}

	if (port->int_sts.task_file_error) error("ATA identify task file error 2");

	// Stringek letisztítása (A betűk visszacserélése és NUL terminátor beillesztése)
	// Minden második betű az azelőttivel fel van cserélve
	for (u32 j = 0; j < 20; j += 2) {
		char tmp = id->model[j];
		id->model[j] = id->model[j+1];
		id->model[j+1] = tmp;
	}
	for (u8 i = sizeof(id->model)-1; i != 255; i--) {
		if (id->model[i] == ' ')
			id->model[i] = 0;
		else
			break;
	}

	for (u8 i = sizeof(id->firmware)-1; i != 255; i--) {
		if (id->firmware[i] == ' ')
			id->firmware[i] = 0;
		else
			break;
	}
	for (u32 j = 0; j < 4; j += 2) {
		char tmp = id->firmware[j];
		id->firmware[j] = id->firmware[j+1];
		id->firmware[j+1] = tmp;
	}

	for (u8 i = sizeof(id->serial)-1; i != 255; i--) {
		if (id->serial[i] == ' ')
			id->serial[i] = 0;
		else
			break;
	}
	for (u32 j = 0; j < 10; j += 2) {
		char tmp = id->serial[j];
		id->serial[j] = id->serial[j+1];
		id->serial[j+1] = tmp;
	}

	return id;
}
