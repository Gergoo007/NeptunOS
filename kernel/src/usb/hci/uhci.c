#include <usb/hci/uhci.h>

uhci_t* uhcis = NULL;
u64 num_uhcis;

void uhci_stop(uhci_t* hc) {
	u16 cmd = inw(hc->io + REG_CMD);
	cmd &= ~CMD_RS;
	outw(cmd, hc->io + REG_CMD);
}

void uhci_run(uhci_t* hc) {
	u16 cmd = inw(hc->io + REG_CMD);
	outw(cmd | CMD_RS, hc->io + REG_CMD);
}

void uhci_on_dev(uhci_t* hc, u8 ls) {
	if (!usb_devs) usb_devs = request_page();

	usb_desc_dev_t* dev_desc = pmm_alloc_page();
	usb_request_t setup = {
		.req_type = 0x80,
		.req = 6,
		.value = 0 | (USB_DESC_DEVICE << 8),
		.index = 0,
		.length = 18,
	};
	void* packet = pmm_alloc_page();
	memcpy(&setup, (u64)packet, 8);
	ptr_32bit(packet);
	uhci_send_in(hc, &((usb_dev_t) { .addr = 0, .endp = 0, .ls = ls }), packet, dev_desc, 18);

	// Manufacturer hossz majd string lekérdezése
	usb_desc_string_t* string_desc = pmm_alloc_page();
	// US English: 0x0409
	usb_request_t req = {
		.req_type = 0x80,
		.req = 6,
		.value = (USB_DESC_STRING << 8) | dev_desc->manufacturer_index,
		.index = 0x0409,
		.length = 1,
	};
	memcpy(&req, (u64)packet, 8);
	uhci_send_in(hc, &((usb_dev_t) { .addr = 0, .endp = 0, .ls = ls }), packet, string_desc, 1);

	u8 manlen = string_desc->len;
	req.length = manlen;
	memcpy(&req, (u64)packet, 8);
	uhci_send_in(hc, &((usb_dev_t) { .addr = 0, .endp = 0, .ls = ls }), packet, string_desc, manlen);
	char* manufacturer = request_page();
	utf16_to_asciin(string_desc->string, manufacturer, (manlen - 2) / 2);

	// Product hossz majd string lekérdezése
	req.value = (USB_DESC_STRING << 8) | dev_desc->product_index,
	req.length = 1;
	memcpy(&req, (u64)packet, 8);
	uhci_send_in(hc, &((usb_dev_t) { .addr = 0, .endp = 0, .ls = ls }), packet, string_desc, 1);

	u8 prodlen = string_desc->len;
	req.length = prodlen;
	memcpy(&req, (u64)packet, 8);
	uhci_send_in(hc, &((usb_dev_t) { .addr = 0, .endp = 0, .ls = ls }), packet, string_desc, prodlen);
	char* product = manufacturer + manlen + 1;
	utf16_to_asciin(string_desc->string, product, (prodlen - 2) / 2);

	pmm_free_page(packet);

	// Address beállítása
	u8 addr = bm_find_free(&addr_bm);
	bm_set(&addr_bm, addr, 1); // Mostmár foglalt
	// Hozzá kell adni 1-et mert a 0-ás cím már foglalt
	addr++;

	// Address küldése az eszköznek
	uhci_send_address(hc, &((usb_dev_t) { .addr = 0, .endp = 0, .ls = ls }), addr);

	// Eszköz hozzáadása a listához
	usb_devs[num_usb_devs].addr = addr;
	usb_devs[num_usb_devs].endp = 0; // TODO: chnage
	usb_devs[num_usb_devs].hc = hc;
	usb_devs[num_usb_devs].hc_type = HC_UHCI;
	usb_devs[num_usb_devs].ls = ls;
	usb_devs[num_usb_devs].manufacturer = manufacturer;
	usb_devs[num_usb_devs].product = product;
	num_usb_devs++;
}

void uhci_init_controller(pci_hdr_t* device) {
	if (!addr_bm.addr) {
		addr_bm.addr = (u64)request_page();
		addr_bm.size = 128;
		bm_init(&addr_bm);
	}

	if (!uhcis) {
		uhcis = request_page();
		memset(uhcis, 0, 0x1000);
	}

	uhci_t* hc = &uhcis[num_uhcis];
	num_uhcis++;

	hc->hdr = device;
	hc->io = device->type0.bar4 & ~((u32) 0b11);

	// Global reset
	u16 cmd = inw(hc->io + REG_CMD);
	cmd |= CMD_GRESET;
	outw(cmd, hc->io + REG_CMD);
	sleep(15);
	cmd = inw(hc->io + REG_CMD);
	cmd &= ~CMD_GRESET;
	outw(cmd, hc->io + REG_CMD);

	// USB megszakítások kikapcsolása
	outw(0, hc->io + REG_INTR);

	// Frame list létrehozása
	hc->frame_list = pmm_alloc_page();
	checkalign(hc->frame_list, 0x1000);
	ptr_32bit(hc->frame_list);

	// + kitöltése
	for (u16 i = 0; i < 1024; i++) {
		*(hc->frame_list + i) = TD_LPTR_TERM;
	}

	outl((u32)(u64)hc->frame_list, hc->io + REG_FRBASEADD);
	outw(0, hc->io + REG_FRNUM);
	outb(0x40, hc->io + REG_SOFMOD);

	// STS kitisztítása
	outw(0xffff, hc->io + REG_STS);

	// Futtatás
	cmd = inw(hc->io + REG_CMD);
	cmd |= CMD_RS;
	outw(cmd, hc->io + REG_CMD);

	// Port 1 vizsgálata
	u16 port = inw(hc->io + REG_PORT1);
	// Ha nincs rákötve semmi akkor kihagyja
	if (!(port & PORT_CONN)) goto port2;

	// Port bekapcs.
	port |= PORT_ENABLED;
	outw(port, hc->io + REG_PORT1);
	sleep(50);

	// Eszköz reset
	port = inw(hc->io + REG_PORT1);
	u8 ls = (port & PORT_LOWSP) ? 1 : 0;

	port |= PORT_DEV_RESET;
	outw(port, hc->io + REG_PORT1);
	sleep(10);
	port &= ~PORT_DEV_RESET;
	outw(port, hc->io + REG_PORT1);

	// Eszköz vizsgálata
	uhci_on_dev(hc, ls);

port2:
	// Port 2 vizsgálata
	port = inw(hc->io + REG_PORT2);
	// Ha nincs rákötve semmi akkor kihagyja
	if (!(port & PORT_CONN)) return;

	// Port bekapcs.
	port |= PORT_ENABLED;
	outw(port, hc->io + REG_PORT2);
	sleep(50);

	// Eszköz reset
	port = inw(hc->io + REG_PORT2);
	ls = (port & PORT_LOWSP) ? 1 : 0;

	port |= PORT_DEV_RESET;
	outw(port, hc->io + REG_PORT2);
	sleep(10);
	port &= ~PORT_DEV_RESET;
	outw(port, hc->io + REG_PORT2);

	// Eszköz vizsgálata
	uhci_on_dev(hc, ls);
}

void uhci_send_in(uhci_t* hc, usb_dev_t* dev, u8* packet, void* output, u8 len) {
	ptr_32bit(output);
	ptr_32bit(packet);

	// SETUP packet küldése
	// Utána IN
	// Végül OUT

	if (!hc->qhs) {
		hc->qhs = pmm_alloc_page();
		ptr_32bit(hc->qhs);
		checkalign(&hc->qhs[2], 16);
		memset(hc->qhs, 0, 0x1000);
	}

	if (!hc->tds) {
		hc->tds = pmm_alloc_page();
		ptr_32bit(hc->tds);
		checkalign(&hc->tds[2], 32);
		memset(hc->tds, 0, 0x1000);
	}

	// SETUP TD
	hc->tds[0].buffer_ptr = (u32)(u64)packet;
	hc->tds[0].link_ptr = (u32)(u64)&hc->tds[1] | TD_LPTR_DEPTH;
	hc->tds[0]._ctrl_sts.ioc = 0; // ha nem jó
	hc->tds[0]._ctrl_sts.ls = dev->ls;
	hc->tds[0]._ctrl_sts.active = 1;
	hc->tds[0]._token.addr = dev->addr;
	hc->tds[0]._token.endp = dev->endp;
	hc->tds[0]._token.data = 0;
	hc->tds[0]._token.maxlen = 7;
	hc->tds[0]._token.pid = TD_PACKET_SETUP;

	// IN TD-k
	i8 remaining = len;
	u8 i = 0;
	u8 data = 0;
	while (remaining > 0) {
		data = !data;

		hc->tds[i+1].buffer_ptr = (u32)(u64)output+(i*8);
		hc->tds[i+1].link_ptr = (u32)(u64)&hc->tds[i+2] | TD_LPTR_DEPTH;
		hc->tds[i+1]._ctrl_sts.ioc = 0;
		hc->tds[i+1]._ctrl_sts.ls = dev->ls;
		hc->tds[i+1]._ctrl_sts.active = 1;
		hc->tds[i+1]._token.addr = dev->addr;
		hc->tds[i+1]._token.endp = dev->endp;
		hc->tds[i+1]._token.data = data;
		hc->tds[i+1]._token.maxlen = 7;
		hc->tds[i+1]._token.pid = TD_PACKET_IN;

		remaining -= 8;
		i++;
	}
	
	// STATUS
	hc->tds[i+1].buffer_ptr = 0;
	hc->tds[i+1].link_ptr = (u32)(u64)&hc->tds[i+2] | TD_LPTR_DEPTH;
	hc->tds[i+1]._ctrl_sts.ioc = 1;
	hc->tds[i+1]._ctrl_sts.ls = dev->ls;
	hc->tds[i+1]._ctrl_sts.active = 1;
	hc->tds[i+1]._token.addr = dev->addr;
	hc->tds[i+1]._token.endp = dev->endp;
	hc->tds[i+1]._token.data = 1;
	hc->tds[i+1]._token.maxlen = 0x7ff;
	hc->tds[i+1]._token.pid = TD_PACKET_OUT;

	hc->tds[i+2].link_ptr = TD_LPTR_TERM | TD_LPTR_DEPTH;

	// QH létrehozása
	hc->qhs[0].element = (u32)(u64)&hc->tds[0];
	hc->qhs[0].head = TD_LPTR_TERM;

	// QH hozzáadása, futtatása
	hc->frame_list[0] = (u32)(u64)&hc->qhs[0] | TD_LPTR_QH;
	uhci_stop(hc);
	outw(0, hc->io + REG_FRNUM);
	uhci_run(hc);

	// Várakozás amíg el nem készül
	while (!(inw(hc->io + REG_STS) & 1)) sleep(1);

	outw(0xffff, hc->io + REG_STS);
}

void uhci_send_address(uhci_t* hc, usb_dev_t* dev, u8 addr) {
	void* packet = pmm_alloc_page();
	ptr_32bit(packet);

	usb_request_t req = {
		.req_type = 0,
		.req = 5,
		.value = addr,
		.index = 0,
		.length = 0,
	};

	memcpy(&req, (u64)packet, 8);

	// SETUP packet küldése
	// Végül STATUS

	if (!hc->qhs) {
		hc->qhs = pmm_alloc_page();
		ptr_32bit(hc->qhs);
		checkalign(&hc->qhs[2], 16);
		memset(hc->qhs, 0, 0x1000);
	}

	if (!hc->tds) {
		hc->tds = pmm_alloc_page();
		ptr_32bit(hc->tds);
		checkalign(&hc->tds[2], 32);
		memset(hc->tds, 0, 0x1000);
	}

	// SETUP TD
	hc->tds[0].buffer_ptr = (u32)(u64)packet;
	hc->tds[0].link_ptr = (u32)(u64)&hc->tds[1] | TD_LPTR_DEPTH;
	hc->tds[0]._ctrl_sts.ioc = 0; // ha nem jó
	hc->tds[0]._ctrl_sts.ls = dev->ls;
	hc->tds[0]._ctrl_sts.active = 1;
	hc->tds[0]._token.addr = dev->addr;
	hc->tds[0]._token.endp = dev->endp;
	hc->tds[0]._token.data = 0;
	hc->tds[0]._token.maxlen = 7;
	hc->tds[0]._token.pid = TD_PACKET_SETUP;
	
	// STATUS
	hc->tds[1].buffer_ptr = 0;
	hc->tds[1].link_ptr = (u32)(u64)&hc->tds[2] | TD_LPTR_DEPTH;
	hc->tds[1]._ctrl_sts.ioc = 1;
	hc->tds[1]._ctrl_sts.ls = dev->ls;
	hc->tds[1]._ctrl_sts.active = 1;
	hc->tds[1]._token.addr = dev->addr;
	hc->tds[1]._token.endp = dev->endp;
	hc->tds[1]._token.data = 1;
	hc->tds[1]._token.maxlen = 0x7ff;
	hc->tds[1]._token.pid = TD_PACKET_IN;

	hc->tds[2].link_ptr = TD_LPTR_TERM | TD_LPTR_DEPTH;

	// QH létrehozása
	hc->qhs[0].element = (u32)(u64)&hc->tds[0];
	hc->qhs[0].head = TD_LPTR_TERM;

	// QH hozzáadása, futtatása
	hc->frame_list[0] = (u32)(u64)&hc->qhs[0] | TD_LPTR_QH;
	uhci_stop(hc);
	outw(0, hc->io + REG_FRNUM);
	uhci_run(hc);

	// Várakozás amíg el nem készül
	while (!(inw(hc->io + REG_STS) & 1)) sleep(1);

	outw(0xffff, hc->io + REG_STS);
}
