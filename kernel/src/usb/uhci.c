#include <usb/uhci.h>

void uhci_init_controller(pci_hdr_t* device) {
	u16 io = device->type0.bar4 & ~((u32) 0b11);

	// Global reset
	u16 cmd = inw(io + REG_CMD);
	cmd |= CMD_GRESET;
	outw(cmd, io + REG_CMD);
	sleep(15);
	cmd = inw(io + REG_CMD);
	cmd &= ~CMD_GRESET;
	outw(cmd, io + REG_CMD);

	// USB megszakítások kikapcsolása
	outw(0, io + REG_INTR);

	// Frame list létrehozása
	u32* frame_list = pmm_alloc_page();
	checkalign(frame_list, 0x1000);
	ptr_32bit(frame_list);

	// + kitöltése
	for (u16 i = 0; i < 1024; i++) {
		*(frame_list + i) = TD_LPTR_TERM;
	}

	outl((u32)(u64)frame_list, io + REG_FRBASEADD);
	outw(0, io + REG_FRNUM);
	outb(0x40, io + REG_SOFMOD);

	// Futtatás
	cmd = inw(io + REG_CMD);
	cmd |= CMD_RS;
	outw(cmd, io + REG_CMD);

	// Port 1 vizsgálata
	u16 port1 = inw(io + REG_PORT1);
	// Ha nincs rákötve semmi akkor kihagyja
	if (!(port1 & PORT_CONN)) goto port2;

	printk("Found device on port!\n");

	// Port bekapcs.
	port1 |= PORT_ENABLED;
	outw(port1, io + REG_PORT1);
	sleep(50);

	// Eszköz reset
	port1 = inw(io + REG_PORT1);
	u8 ls = (port1 & PORT_LOWSP) ? 1 : 0;

	port1 |= PORT_DEV_RESET;
	outw(port1, io + REG_PORT1);
	sleep(10);
	port1 &= ~PORT_DEV_RESET;
	outw(port1, io + REG_PORT1);

	u8 setup_packet[8] = { 0x80, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08 };

	// SETUP packet küldése
	// Utána IN
	// Végül OUT
	uhci_qh_t* qhs = pmm_alloc_page();
	ptr_32bit(qhs);
	checkalign(&qhs[2], 16);
	memset(qhs, 0, 0x1000);

	uhci_td_t* tds = pmm_alloc_page();
	ptr_32bit(tds);
	checkalign(&qhs[2], 32);
	memset(tds, 0, 0x1000);

	void* base = pmm_alloc_page();
	ptr_32bit(base);
	void* setup_storage = base;
	void* in_storage = base + 2048;

	memcpy(setup_packet, (u64)setup_storage, 8);

	// SETUP TD
	tds[0].buffer_ptr = (u32)(u64)setup_storage;
	tds[0].link_ptr = (u32)(u64)&tds[1] | TD_LPTR_DEPTH;
	tds[0]._ctrl_sts.ioc = 1; // ha nem jó
	tds[0]._ctrl_sts.ls = ls;
	tds[0]._ctrl_sts.active = 1;
	tds[0]._token.addr = 0;
	tds[0]._token.endp = 0;
	tds[0]._token.data = 0;
	tds[0]._token.maxlen = 7;
	tds[0]._token.pid = TD_PACKET_SETUP;

	// IN TD
	tds[1].buffer_ptr = (u32)(u64)in_storage;
	tds[1].link_ptr = TD_LPTR_TERM | TD_LPTR_DEPTH;
	tds[1]._ctrl_sts.ioc = 1; // ha nem jó
	tds[1]._ctrl_sts.ls = ls;
	tds[1]._ctrl_sts.active = 1;
	tds[1]._token.addr = 0;
	tds[1]._token.endp = 0;
	tds[1]._token.data = 1;
	tds[1]._token.maxlen = 7;
	tds[1]._token.pid = TD_PACKET_IN;

	// QH létrehozása
	qhs[0].element = (u32)(u64)&tds[0];
	qhs[0].head = TD_LPTR_TERM;

	cmd = inw(io + REG_CMD);
	cmd &= ~CMD_RS;
	outw(cmd, io + REG_CMD);

	sleep(50);

	// QH hozzáadása
	frame_list[0] = (u32)(u64)&qhs[0] | TD_LPTR_QH;

	outw(0, io + REG_FRNUM);

	// STS kitisztítása
	outw(0xffff, io + REG_STS);

	cmd = inw(io + REG_CMD);
	cmd |= CMD_RS;
	outw(cmd, io + REG_CMD);

	sleep(100);

	// Eredmények
	printk("cmd: %04x\n", inw(io + REG_CMD));
	printk("status: %04x\n", inw(io + REG_STS));
	printk("setup:..%02x\n", tds[0]._ctrl_sts.status);
	printk("in:.....%02x\n", tds[1]._ctrl_sts.status);
	printk("status:.%02x\n", tds[2]._ctrl_sts.status);

	printk("buffer: %p\n", *(u64*)in_storage);
	usb_desc_dev_t* response = in_storage;
	printk("len %d\n", response->len);
	printk("type %d\n", response->desc_type);
	printk("USB ver %04x\n", response->usb_ver);
	printk("class %d\n", response->class);
	printk("subcl %d\n", response->subclass);
	printk("prot %d\n", response->protocol);
	printk("max size %d\n", response->max_packet_size);

port2:
	asm ("nop");
}
