#include <gfx/console.hh>
#include <devmgr/module.hh>
#include <devmgr/devmgr.hh>
#include <pci/pci.hh>
#include <arch/amd64/amd64.hh>
#include <arch/amd64/io.hh>
#include <arch/amd64/paging.hh>
#include <devmgr/usb/usb.hh>

#include "ehci.hh"

__attribute__((section(".modinfo"), used))
volatile constexpr module_metadata_t _modinfo {
	.name = "ehci",
	.triggertype = ModuleTriggerTypes::PCI_CLASS_SUBCLASS_PROGIF,
	.trigger = { .PCI_CLASS_SUBCLASS_PROGIF { 0xc, 0x3, 0x20 } }
};

static bool hchalted() { return EhciRegs::USBSTS.read().USBSTS.hchalted; }
static bool isrunning() { return EhciRegs::USBCMD.read().USBCMD.running; }
static void setrunning(bool running) {
	auto cmd = EhciRegs::USBCMD.read().USBCMD;
	cmd.running = running;
	EhciRegs::USBCMD.write(cmd);
	arch_start_timer();
	while (EhciRegs::USBSTS.read().USBSTS.hchalted == running)
		if (arch_elapsed(100))
			fatal("Controller refusing to start running!");
}
static void hcreset() {
	// HCReset
	EhciRegs::USBCMD.write(0b10);
	arch_start_timer();
	while (EhciRegs::USBCMD.read().USBCMD.hcreset) {
		if (arch_elapsed(100))
			fatal("HC not coming out of HCReset!");
	}
}

static u64 pool;
static bitmap_t bm;
static constexpr u32 unitsize = 64;
static bool bm_init = false;

[[nodiscard]]
static ehci_qtd* ehci_alloc_qtd() {
	u64 addr = pool + bm.find_and_set() * unitsize;
	memset((void*)addr, 0, unitsize);
	if constexpr (debug)
		assert((PHYSICAL(addr) >> 32) == 0);
	return (ehci_qtd*)addr;
}

[[nodiscard]]
static ehci_qh* ehci_alloc_qh() {
	u64 addr = pool + bm.find_and_set() * unitsize;
	memset((void*)addr, 0, unitsize);
	if constexpr (debug)
		assert((PHYSICAL(addr) >> 32) == 0);
	return (ehci_qh*)addr;
}

[[nodiscard]]
static void* ehci_alloc_page() {
	u64 addr = (u64)pmm_alloc();
	memset((void*)addr, 0, pmm_pagesize);
	if constexpr (debug)
		assert((PHYSICAL(addr) >> 32) == 0);
	return (void*)addr;
}

template <typename T>
[[nodiscard]]
static u32 elookup(T* turi) {
	u64 i = paging_lookup((u64)turi);
	if constexpr (debug) assert((i >> 32) == 0);
	return i & 0xffffffff;
}

// Minden kernel interfész procedúrának BE KELL EZT ÁLLÍTANIA!
ehci_internal context;
static void ehci_update_context(device_t& hc) {
	u64 addr = pci_read(hc, PciRegs::BAR0), orig = addr;
	assert((addr & 1) == 0);
	addr &= ~0b1111;

	pci_write(hc, PciRegs::BAR0, 0xffffffff);
	u64 size = (~(pci_read(hc, PciRegs::BAR0) & ~0xf)) + 1;
	pci_write(hc, PciRegs::BAR0, orig);

	for (u32 i = 0; i < align(size, 0x1000); i += 0x1000)
		map_page(VIRTUAL(addr) + i, addr + i, 0b11, MCACHE::UC);

	context.mmio = VIRTUAL(addr);
	context.caplength = *(volatile u8*)context.mmio;
	if (context.caplength != 0x20)
		warn("Gyanus CAPLENGTH: %02x", context.caplength);
}

static void insert_qh(ehci_qh* qh) {
	qh->horiz_link.ptr = context.head->horiz_link.ptr;
	assert(qh->horiz_link.type == EhciQHType::QH);
	context.head->horiz_link.ptr = elookup(qh) | ((u32)EhciQHType::QH << 1);

	report("Inserted QH %x", elookup(qh));
}

void ehci_send2(device_t& usbdev, u8 addr, u8 endp, usb_request* request, void* databuf, u64 size) {

}

void ehci_send_reset2(device_t& usbdev) {

}

void* response = nullptr;

static void init_port(u8 portnum) {
	if (hchalted()) {
		error("HCHalted is still set, not ready for port enumeration");
		fatal("USBSTS %08x USBCMD %08x", EhciRegs::USBSTS.read()._raw, EhciRegs::USBCMD.read()._raw);
	}

	auto reg = ehci_reg(portnum);
	auto port = reg.read().PORTSC;

	port.port_enabled_change = 1;
	port.overcurrent_change = 1;
	port.connect_sts_change = 1;
	port.port_pwr = 1;
	reg.write(port);
	port = reg.read().PORTSC;

	arch_sleep(10);
	port.reset = 1;
	port.port_enabled = 0;
	port.port_pwr = 1;
	reg.write(port);
	arch_sleep(50);

	port = reg.read().PORTSC;

	port.reset = 0;
	reg.write(port);
	arch_sleep(3);

	port = reg.read().PORTSC;
	assert(!port.reset);

	if (port.connect_sts) {
		warn("Eszkoz itt: port %d; line status %02b, port enabled: %s", portnum, port.line_sts, port.port_enabled ? "true" : "false");
	}

	if (!port.port_enabled) return;

	ehci_qtd* td0 = ehci_alloc_qtd();
	ehci_qtd* td1 = ehci_alloc_qtd();
	ehci_qtd* td2 = ehci_alloc_qtd();

	response = ehci_alloc_page();
	usb_request* req = (usb_request*)ehci_alloc_page();
	req->bmRequestType = 0x80;
	req->bRequest = UsbRequests::GET_DESCRIPTOR;
	req->wValue = (1 << 8) | 0;
	req->wIndex = 0;
	req->wLength = 8;

	td0->buffers[0] = elookup(req);
	td0->buffers[1] = td0->buffers[0] + 0x1000;
	td0->buffers[2] = td0->buffers[1] + 0x1000;
	td0->buffers[3] = td0->buffers[2] + 0x1000;
	td0->buffers[4] = td0->buffers[3] + 0x1000;
	td0->alt_next_qtd.t = 1;
	td0->next_qtd.ptr = elookup(td1);
	td0->token.bytes = 8;
	td0->token.current_page = 0;
	td0->token.data = 0;
	td0->token.err_counter = 3;
	td0->token.ioc = 0;
	td0->token.pid = EhciPid::SETUP;
	td0->token.sts.raw = 0x80;

	td1->buffers[0] = elookup(response);
	td1->buffers[1] = td1->buffers[0] + 0x1000;
	td1->buffers[2] = td1->buffers[1] + 0x1000;
	td1->buffers[3] = td1->buffers[2] + 0x1000;
	td1->buffers[4] = td1->buffers[3] + 0x1000;
	td1->next_qtd.t = 1;
	td1->alt_next_qtd.ptr = elookup(td2);
	td1->token.bytes = 64;
	td1->token.current_page = 0;
	td1->token.data = 1;
	td1->token.err_counter = 3;
	td1->token.ioc = 0;
	td1->token.pid = EhciPid::IN;
	td1->token.sts.raw = 0x80;

	td2->alt_next_qtd.ptr = 1;
	td2->next_qtd.ptr = 1;
	td2->token.bytes = 0;
	td2->token.current_page = 0;
	td2->token.data = 0;
	td2->token.err_counter = 3;
	td2->token.ioc = 1;
	td2->token.pid = EhciPid::OUT;
	td2->token.sts.raw = 0x80;

	ehci_qh* qh = ehci_alloc_qh();
	
	// ????
	// qh->overlay = *td0;
	qh->current_qtd = 0;
	qh->overlay.next_qtd.ptr = elookup(td0);

	qh->endpoint_caps.high_bw_pipe_multiplier = EhciTAPerUframe::ONE_PER_UFRAME;
	qh->endpoint_caps.hub_addr = 0;
	qh->endpoint_caps.int_sched_mask = 0;
	qh->endpoint_caps.split_completion_mask = 0;
	qh->endpoint_caps.port_num = 0;

	qh->endpoint_characteristics.ctl_endpoint = 0;
	qh->endpoint_characteristics.endpoint = 0;
	qh->endpoint_characteristics.endpoint_speed = EhciEndpointSpeed::HS;
	qh->endpoint_characteristics.addr = 0;
	qh->endpoint_characteristics.data_toggle_ctl = 0;
	qh->endpoint_characteristics.inactive_on_success = 0;
	qh->endpoint_characteristics.mps = 64;
	qh->endpoint_characteristics.nak_reload_counter = 4;

	insert_qh(qh);
	report("reclaim %x", context.head->horiz_link.ptr);

	// while (td2->token.sts.active);
	arch_start_timer();
	while (!EhciRegs::USBSTS.read().USBSTS.usbint) {
		if (arch_elapsed(500)) {
			error("Transaction didn't occur, timeout!");
			report("TDs finished, values: %02x, %02x, %02x", td0->token.sts.raw, td1->token.sts.raw, td2->token.sts.raw);
			report("QH state: %08x %08x %08x", qh->current_qtd, qh->overlay.next_qtd, qh->overlay.token);
			report("%08x %08x", EhciRegs::USBCMD.read()._raw, EhciRegs::USBSTS.read()._raw);
			pause();
		}
	}

	report("TDs finished, values: %02x, %02x, %02x", td0->token.sts.raw, td1->token.sts.raw, td2->token.sts.raw);

	usb_descriptor_device* d = (usb_descriptor_device*)response;
	report("Kesz a transaction: %d", d->bLength);

	arch_sleep(1000);

	pause();
}

// Software must not modify the Asynchronous Schedule Enable bit unless the value of the Asynchronous Schedule
// Enable bit equals that of the Asynchronous Schedule Status bit

// Async queue feldolgozásakor ha a recl. head bit 1, a USBSTS.Reclamation pedig 0,
// a HC azonnal abbahagyja az async schedule végrehajtását

extern "C" void mod_main(device_t& dev) {
	if (!bm_init) {
		pool = (u64)pmm_alloc();
		bm.init(pmm_alloc(), 4096 * 8 / 32);
		bm_init = true;
	}

	ehci_update_context(dev);

	report("EHCI detected @ %p", (void*)context.mmio);
	u32 pcicmd = pci_read(dev, PciRegs::CMD);
	pcicmd |= 0b100; // bus master
	pcicmd |= 0b010; // mem access
	pci_write(dev, PciRegs::CMD, pcicmd);

	auto hcsp = EhciRegs::HCSPARAMS.read().HCSPARAMS;
	auto eecp = EhciRegs::HCCPARAMS.read().HCCPARAMS.extended_caps_ptr;
	if (eecp) {
		pci_register_t usblegsupreg(eecp, 32);
		USBLEGSUP usblegsup = pci_read(dev, usblegsupreg);
		usblegsup.hc_os_owned = true;
		pci_write(dev, usblegsupreg, usblegsup.raw);
		arch_sleep(1);
		usblegsup = pci_read(dev, usblegsupreg);
		report("hc ownership (bios, os): %d, %d", usblegsup.hc_bios_owned, usblegsup.hc_os_owned);
	} else {
		warn("No EHCI USB legacy support!");
	}

	auto cmd = EhciRegs::USBCMD.read().USBCMD;
	cmd.running = false;
	EhciRegs::USBCMD.write(cmd);

	cmd = EhciRegs::USBCMD.read().USBCMD;
	cmd.hcreset = 1;
	EhciRegs::USBCMD.write(cmd);

	arch_start_timer();
	while (EhciRegs::USBCMD.read().USBCMD.hcreset)
		if (arch_elapsed(100))
			fatal("hcreset not working");

	EhciRegs::USBINTR.write(7);
	EhciRegs::FRINDEX.write(0);
	EhciRegs::CTRL4GSEGMENT.write(0);

	EhciRegs::CONFIGFLAG.write(1);

	if (hcsp.port_pwr_ctl_supported)
		warn("Port power control supported!");
	if (hcsp.port_indicator_supported)
		warn("Port indicator supported!");

	context.head = ehci_alloc_qh();
	context.head->endpoint_characteristics.head_of_reclamation_list = true;

	// Dummy qTD
	ehci_qtd* dummy = ehci_alloc_qtd();
	dummy->token.sts.active = false;
	dummy->next_qtd.t = 1;
	dummy->alt_next_qtd.t = 1;
	report("Dummy qTD is %x; Dummy QH is %x", elookup(dummy), elookup(context.head));

	context.head->current_qtd = 0;
	context.head->overlay.next_qtd.ptr = elookup(dummy);

	context.head->horiz_link.ptr = elookup(context.head);
	context.head->horiz_link.type = EhciQHType::QH;

	EhciRegs::ASYNCLISTBASE.write(elookup(context.head));

	cmd = EhciRegs::USBCMD.read().USBCMD;
	cmd.async_schedule_enable = true;
	cmd.running = true;
	EhciRegs::USBCMD.write(cmd);

	arch_start_timer();
	while (!EhciRegs::USBSTS.read().USBSTS.async_schedule)
		if (arch_elapsed(100))
			fatal("HC refused to start async schedule!");

	while (EhciRegs::USBSTS.read().USBSTS.hchalted)
		if (arch_elapsed(100))
			fatal("HC refused to clear HCHalted!");

	report("num ports: %d; num companions: %d", hcsp.num_ports, hcsp.num_companions);
	for (u32 i = 0; i < hcsp.num_ports; i++)
		init_port(i);
}
