#include <types.hh>
#include <devmgr/module.hh>
#include <devmgr/devmgr.hh>
#include <devmgr/msd/msd.hh>
#include <util/ata.hh>
#include <pci/pci.hh>
#include <arch/arch.hh>
#include <arch/amd64/paging.hh>
#include <mm/vmm.hh>
#include <cppcompat.hh>

#include "ahci.hh"

// TODO: Staggered spin up
// TODO: CMD HDR -> Prefetchable

constexpr u32 SIG_ATA = 0x00000101;
constexpr u32 SIG_ATAPI = 0xEB140101;
constexpr u32 SIG_SEMB = 0xC33C0101; // enclosure mgmt bridge
constexpr u32 SIG_PM = 0x96690101; // port multipl

// static_assert(PMM_PAGESIZE == mibs2bytes(2));
// 2 MiB-os pool, ebből:
// 1024 + 256 + 128*32 + 16*32 -> 5376 byte kell az alapvető struktúráknak, így marad
// 2 091 776 byte a PRDT-knek, így jut 4080 PRDT mindegyik command table-nek
// (4085,5 lenne de a command table-knek 128 byte-os határon kell elhelyezkedniük)
// 4080 PRDT-vel 65 408 byte mindegyik command table
// Ennyi PRDT alkalmas 16 GiB-nyi adat egyidejű mozgatására minusz 4 MiB a kezdeti page alignment miatt
constexpr u32 PRDTL = 4080;
constexpr u32 CMD_TBL_SIZE = PRDTL * 16 + 128;

__attribute__((section(".modinfo"), used))
volatile constexpr module_metadata_t _modinfo {
	.name = "ahci",
	.triggertype = ModuleTriggerTypes::PCI_CLASS_SUBCLASS,
	.trigger = { .PCI_CLASS_SUBCLASS { 0x01, 0x06 } }
};

pstruct ahci_internal {
	volatile hba_regs* r;
};

static u32 alloc_cmdslot(volatile hba_regs* r, u32 port) {
	auto& p = r->ports[port];
	u32 slotbitmap = p.cmd_issue | p.sata_active;
	for (u32 i = 0; i < 32; i++) {
		if ((slotbitmap & (1 << i)) == 0)
			return i;
	}
	return -1u;
}

#define poll(expr, timeout) \
	do { \
		arch_start_timer(); \
		while ((expr)) \
			if (arch_elapsed(timeout)) \
				fatal("Timeout reached on expression '%s' (%s:%d)", #expr, __FILE__, __LINE__); \
	} while (false);

static u32 readl(volatile u32* addr) { return *(volatile u32*)addr; }

static void setrunning(volatile hba_regs* r, u32 i, bool run) {
	volatile u32& cmd = *(u32*)&r->ports[i].cmd_sts;
	volatile u32& tfd = *(u32*)&r->ports[i].taskfile;

	if (run) {
		cmd |= (1u << PxCMD_FRE);
		poll(!(cmd & (1 << PxCMD_FR)), 100);
		cmd |= (1u << PxCMD_ST);
		poll(!(cmd & (1 << PxCMD_CR)), 100);
	} else {
		cmd &= ~(1u << PxCMD_ST);
		poll((cmd & (1 << PxCMD_CR)) || (tfd & (1 << PxTFD_BSY)) || (tfd & (1 << PxTFD_DRQ)), 100);
		// poll(cmd & PxCMD_CR, 100);
		cmd &= ~(1u << PxCMD_FRE);
		poll(cmd & (1 << PxCMD_FR), 100);
	}
}

template <typename Lambda>
static bool ahci_send_cmd(Device& dev, Lambda&& create_cmd) {
	auto* r = ((ahci_internal*)dev.parent->extra)->r;
	u32 i = dev.loc;

	u32 slot = alloc_cmdslot(r, i);
	r->ports[i].intr_sts = -1;

	cmd_hdr* hdrs = VIRTUAL((cmd_hdr*)r->ports[i].cmd_list_base);
	hdrs[slot].cmd_fis_len = sizeof(fis_reg_h2d) / 4;
	hdrs[slot].write = 0;

	hdrs[slot].prdtlen = create_cmd(VIRTUAL((cmd_table*)hdrs[slot].cmd_table_base));

	// Meg kell várni amíg a port nem buzi
	ahci_px_tfd tf = r->ports[i].taskfile;
	arch_start_timer();
	while (tf.sts.busy || tf.sts.data_transfer_req) {
		if (arch_elapsed(100)) fatal("AHCI port hung");
		tf = r->ports[i].taskfile;
	}

	auto& port = r->ports[i];

	if (port.sata_err)
		fatal("SATA error preceding: %08x", +port.sata_err);

	port.intr_sts = -1u;
	port.cmd_issue |= (1 << slot);

	arch_start_timer();
	while (port.cmd_issue & (1 << slot)) {
		if (arch_elapsed(500)) {
			error("AHCI cmd_issue bit not getting reset!");
			return false;
		}

		if (port.intr_sts & (1 << 30)) {
			error("Task file error!");
			return false;
		}

		if (port.sata_err) {
			error("SATA error: %08x", +port.sata_err);
			return false;
		}
	}

	return true;
}

void ahci_identify(Device& dev, void* identity) {
	assert(dev.subsys == DevmgrSubsys::MSD);
	assert(isaligned(identity, 512));

	auto* r = ((ahci_internal*)dev.parent->extra)->r;
	u32 i = dev.loc;
	r->ports[i].intr_sts = -1;

	ahci_send_cmd(dev, [identity](cmd_table* tbl) -> u32 {
		fis_reg_h2d* cmdfis = (fis_reg_h2d*)tbl->cmd_fis;
		cmdfis->type = FisTypes::REG_H2D;
		cmdfis->cmd_ctl = 1;	// Command
		cmdfis->cmd = AtaCmds::IDENTIFY_DEVICE;

		cmdfis->lba0 = 0;
		cmdfis->lba1 = 0;
		cmdfis->lba2 = 0;
		cmdfis->device = 0;

		cmdfis->lba3 = 0;
		cmdfis->lba4 = 0;
		cmdfis->lba5 = 0;

		cmdfis->count = 1;
		u32 prdtl = 1;

		// meg kell bizonyosodni róla, hogy ha a heapbe megy az adat, akkor mappelve van
		readl((volatile u32*)identity);
		assert(paging_lookup(identity) != -1ull);

		tbl->entries[0].data_base = (u64)paging_lookup(identity);
		tbl->entries[0].bytes = 511;
		tbl->entries[0].ioc = true;

		return prdtl;
	});
}

void ahci_read(Device& dev, u64 lba, u64 sectors, void* into) {
	assert(dev.subsys == DevmgrSubsys::MSD);
	assert(((u64)into & 511) == 0);
	assert(sectors < PRDTL);

	auto* r = ((ahci_internal*)dev.parent->extra)->r;
	u32 i = dev.loc;
	r->ports[i].intr_sts = -1;

	bool success = ahci_send_cmd(dev, [lba, &into, &sectors](cmd_table* tbl) -> u32 {
		fis_reg_h2d* cmdfis = (fis_reg_h2d*)tbl->cmd_fis;
		cmdfis->type = FisTypes::REG_H2D;
		cmdfis->cmd_ctl = 1;	// Command
		cmdfis->cmd = AtaCmds::READ_DMA_EXT;

		cmdfis->lba0 = (lba >> 0) & 0xff;
		cmdfis->lba1 = (lba >> 8) & 0xff;
		cmdfis->lba2 = (lba >> 16) & 0xff;
		cmdfis->device = 1 << 6;	// LBA mode

		cmdfis->lba3 = (lba >> 24) & 0xff;
		cmdfis->lba4 = (lba >> 32) & 0xff;
		cmdfis->lba5 = (lba >> 48) & 0xff;

		cmdfis->count = sectors;
		u32 prdtl = sectors;

		u32 j = 0;
		while (sectors) {
			// meg kell bizonyosodni róla, hogy ha a heapbe megy az adat, akkor mappelve van
			readl((volatile u32*)into);
			assert(paging_lookup(into) != -1ull);

			tbl->entries[j].data_base = (u64)paging_lookup(into);
			tbl->entries[j].bytes = 511;
			tbl->entries[j].ioc = false;

			into = (void*)((u64)into + 512);
			sectors--;
			j++;
		}

		tbl->entries[j-1].ioc = true;

		return prdtl;
	});

	if (!success) {
		fatal("ahci_read unsuccessful! lba: %llx, sectors: %llx; output: %p", lba, sectors, into);
	}
}

// Milyen autista állat találta ki ezt a formátumot? És miért nem lehetett egy faszom null terminatort rakni?
void ahci_sanitize(char* str, u32 len) {
	for (u32 i = 0; i < len / 2; i += 2) {
		char tmp = str[i];
		str[i] = str[i + 1];
		str[i + 1] = tmp;
	}

	u32 i = len;
	while (str[--i] == ' ');
	str[i + 1] = 0;
}

static Device* init_port(Device& hba, u32 i) {
	auto* r = ((ahci_internal*)hba.extra)->r;
	auto& p = r->ports[i];

	// Ha nincs bekapolva vagy nincs eszköz, nem kell vele foglalkozni
	if ((p.sata_sts & 0x0f) != 3) return nullptr;
	if (((p.sata_sts >> 8) & 0x0f) != 1) return nullptr;

	switch (p.sign) {
		case SIG_ATA:
			report("found ata dev");
			break;
		case SIG_ATAPI:
			report("found atapi");
			return nullptr;
		case SIG_PM:
			warn("found port multipl");
			return nullptr;
		case SIG_SEMB:
			return nullptr;
		default:
			error("Unknown AHCI signature: %08x", +p.sign);
			return nullptr;
	}

	constexpr u32 POOL_SIZE = mibs2bytes(2);
	u64 pool = (u64)pmm_alloc(POOL_SIZE);
	memset((void*)pool, 0, POOL_SIZE);

	setrunning(r, i, false);

	cmd_hdr* cmdhdrlist = (cmd_hdr*)pool;
	pool += 1024;
	p.cmd_list_base = PHYSICAL((u64)cmdhdrlist);

	void* recfis = (void*)pool;
	assert(((u64)recfis & 255) == 0);
	pool += 256;
	p.fis_list_base = PHYSICAL((u64)recfis);

	// Parancslista kitöltése
	for (u32 j = 0; j < 32; j++) {
		cmd_hdr* hdr = &cmdhdrlist[j];
		hdr->prdtlen = PRDTL;
		// hdr->clear_buzi_upon_ok = 1;
		hdr->cmd_table_base = PHYSICAL(pool);
		assert((pool & 127) == 0);
		pool += CMD_TBL_SIZE;
	}

	setrunning(r, i, true);

	p.intr_sts = -1;
	p.intr_enable = -1;

	// Új eszközként hozzáadás
	Device& drive = devmgr_add_device(Device {
		.parent = &hba,
		.extra = new msd_call_table { .read = ahci_read },
		.subsys = DevmgrSubsys::MSD,
		.loc = i,
		.kinds = devunion_t(true, device_t_MSD {
			
		}),
	});

	ATA_IDENTITY* id = (ATA_IDENTITY*)kmalloc_aligned(512, 512);
	ahci_identify(drive, id);
	ahci_sanitize((char*)id->ModelNumber, 40);
	ahci_sanitize((char*)id->SerialNumber, 20);

	auto& d = drive.kinds.get<device_t_MSD>();
	d.manufacturerName = String();
	d.productName = String((char*)id->ModelNumber);
	d.serial = String((char*)id->SerialNumber);
	kfree(id);

	return &drive;
}

extern "C" void mod_main(Device& _dev) {
	pci_setup_cmd_reg(_dev);

	auto bar5 = pci_prepare_bar(_dev, 5);
	assert(!bar5.io);
	u64 mmio = bar5.addr;

	volatile hba_regs* r = (hba_regs*)mmio;
	r->bohc |= 0b10; // assert os-owned
	arch_start_timer();
	while (r->bohc & 1) {
		if (arch_elapsed(10)) {
			warn("BIOS didn't release AHCI, forcing takeover...");
			r->bohc &= ~0b1;
			r->bohc |= 0b10;
			break;
		}
	}

	_dev.extra = new ahci_internal(r);

	pci_enable_msi(_dev, 0x60);

	ahci_ghc ghc = r->ghc;
	// ghc.reset = 1;
	// r->ghc = ghc.raw;
	// arch_start_timer();
	// while (ghc.reset & 1) {
	// 	if (arch_elapsed(100)) fatal("AHCI reset took too long");
	// 	ghc = r->ghc;
	// }

	ghc.ahci_enable = 1;
	ghc.intr = 0;
	r->ghc = ghc.raw;

	arch_sleep(10, true);
	ghc = r->ghc;

	ahci_cap caps = r->caps;
	assert(caps.s64bit);

	for (u32 i = 0; i < 32; i++)
		if (r->port_impl & (1 << i))
			init_port(_dev, i);
}
