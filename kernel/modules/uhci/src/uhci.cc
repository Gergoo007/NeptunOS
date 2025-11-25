#include <gfx/console.hh>
#include <devmgr/module.hh>
#include <devmgr/devmgr.hh>
#include <pci/pci.hh>
#include <arch/amd64/amd64.hh>
#include <arch/amd64/io.hh>
#include <devmgr/usb/usb.hh>

#include "uhci.hh"
#include "allocator.hh"

__attribute__((section(".modinfo"), used))
volatile constexpr module_metadata_t _modinfo {
	.name = "uhci",
	.triggertype = ModuleTriggerTypes::PCI_CLASS_SUBCLASS_PROGIF,
	.trigger = { .PCI_CLASS_SUBCLASS_PROGIF { 0xc, 0x3, 0x0 } }
};

static u16 hciio;

u32 uhci_read(const uhci_register& reg) {
	if (reg.bytes == 2)
		return inw(hciio + reg.io);
	else if (reg.bytes == 4)
		return inl(hciio + reg.io);
	else
		fatal("wgat");
	return 0;
}

void uhci_write(const uhci_register& reg, u32 val) {
	if (reg.bytes == 2)
		outw(hciio + reg.io, val);
	else if (reg.bytes == 4)
		outl(hciio + reg.io, val);
	else
		fatal("wgat");
}

void uhci_set_running(bool run) {
	uhci_usbcmd_t cmd;
	cmd.raw = uhci_read(UhciRegs::USBCMD);
	cmd.run = run;
	uhci_write(UhciRegs::USBCMD, cmd.raw);
}

u32 uhci_num_ports() {
	u32 i = 1;
	while (true) {
		u16 val = uhci_read(uhci_register(i));
		if ((val & (1 << 7)) == 0)
			break;

		i++;
	}
	return i - 1;
}

void uhci_send_reset(u8 portnum) {
	uhci_port_t port;
	port.raw = uhci_read(uhci_register(portnum));
	port.reset = 1;
	port.dev_present_ch = 1;
	uhci_write(uhci_register(portnum), port.raw);
	arch_sleep(50);
	port.raw = uhci_read(uhci_register(portnum));
	port.reset = 0;
	port.dev_present_ch = 0;
	port.port_enable = 0;
	uhci_write(uhci_register(portnum), port.raw);

	arch_sleep(1);

	port.raw = uhci_read(uhci_register(portnum));
	assert(port.reset == 0);
	assert(port.port_enable == 0);

	port.dev_present_ch = 1;
	uhci_write(uhci_register(portnum), port.raw);
	port.dev_present_ch = 0;
	port.port_enable = 1;
	uhci_write(uhci_register(portnum), port.raw);

	arch_sleep(1);

	port.raw = uhci_read(uhci_register(portnum));
	assert(port.port_enable);
	port.port_enable = 1;
	port.port_enable_ch = 1;
	port.dev_present_ch = 1;

	uhci_write(uhci_register(portnum), port.raw);

	// Reset recovery
	arch_sleep(200);
}

void uhci_hc_init(device_t* dev) {
	// TODO: legacy support kikapcs?

	// Bus mastering bekapcs
	u32 pcicmd = pci_read(*dev, PciRegs::CMD);
	pcicmd |= 1 << 2;
	pci_write(*dev, PciRegs::CMD, pcicmd);

	report("uhci port: %04x", hciio);
	
	// Stop, vezérlő reset
	uhci_usbcmd_t cmd;
	cmd.raw = uhci_read(UhciRegs::USBCMD);
	cmd.run = 0;
	cmd.hcreset = 1;
	// Minden trazakció után megáll a vezérlő
	cmd.swdebug = 0;
	uhci_write(UhciRegs::USBCMD, cmd.raw);

	// hcreset
	while (uhci_read(UhciRegs::USBCMD) & 2);
	arch_sleep(10);

	// HCHalted
	assert(uhci_read(UhciRegs::USBSTS) & (1 << 5));

	// Reset az eszközökbe
	cmd.raw = uhci_read(UhciRegs::USBCMD);
	cmd.globalreset = 1;
	uhci_write(UhciRegs::USBCMD, cmd.raw);
	arch_sleep(100);
	cmd.raw = uhci_read(UhciRegs::USBCMD);
	cmd.globalreset = 0;
	uhci_write(UhciRegs::USBCMD, cmd.raw);

	// Megszakítások majd ha lesz ACPI akkor
	uhci_write(UhciRegs::USBINTR, 0x0000);

	cmd.raw = uhci_read(UhciRegs::USBCMD);
	uhci_write(UhciRegs::FRNUM, 0x0000);
	if (uhci_read(UhciRegs::FRNUM))
		fatal("Invalid FRNUM: %d; %04x", uhci_read(UhciRegs::FRNUM), cmd.raw);

	uhci_dev_extra_t* hci = (uhci_dev_extra_t*)dev->PCI.extra;

	// Megszakításoknak
	hci->qh1 = uhci_alloc_qh();
	hci->qh2 = uhci_alloc_qh();
	hci->qh4 = uhci_alloc_qh();
	hci->qh8 = uhci_alloc_qh();
	hci->qh16 = uhci_alloc_qh();
	hci->qh32 = uhci_alloc_qh();
	hci->qh64 = uhci_alloc_qh();
	hci->qh128 = uhci_alloc_qh();

	// Kontroll/Bulk tranzakcióknak
	// Külön LS QH kell, mert elvileg a low speed eszközöket később
	// kell turi ippelni
	hci->qhfs = uhci_alloc_qh();
	hci->qhls = uhci_alloc_qh();

	hci->qh128->head.headptr = ulookup(hci->qh64) | 2;
	hci->qh64->head.headptr = ulookup(hci->qh32) | 2;
	hci->qh32->head.headptr = ulookup(hci->qh16) | 2;
	hci->qh16->head.headptr = ulookup(hci->qh8) | 2;
	hci->qh8->head.headptr = ulookup(hci->qh4) | 2;
	hci->qh4->head.headptr = ulookup(hci->qh2) | 2;
	hci->qh2->head.headptr = ulookup(hci->qh1) | 2;
	hci->qh1->head.headptr = ulookup(hci->qhfs) | 2;
	hci->qhfs->head.headptr = ulookup(hci->qhls) | 2;
	hci->qhls->head.t = 1;

	hci->qh128->elem.t = 1;
	hci->qh64->elem.t = 1;
	hci->qh32->elem.t = 1;
	hci->qh16->elem.t = 1;
	hci->qh8->elem.t = 1;
	hci->qh4->elem.t = 1;
	hci->qh2->elem.t = 1;
	hci->qh1->elem.t = 1;
	hci->qhfs->elem.t = 1;
	hci->qhls->elem.t = 1;

	uhci_frame_t* frlist = (uhci_frame_t*)pmm_alloc();
	for (u32 i = 0; i < 1024; i++) {
		if (i % 128 == 0)
			frlist[i].frptr = ulookup(hci->qh128) | 2;
		else if (i % 64 == 0)
			frlist[i].frptr = ulookup(hci->qh64) | 2;
		else if (i % 32 == 0)
			frlist[i].frptr = ulookup(hci->qh32) | 2;
		else if (i % 16 == 0)
			frlist[i].frptr = ulookup(hci->qh16) | 2;
		else if (i % 8 == 0)
			frlist[i].frptr = ulookup(hci->qh8) | 2;
		else if (i % 4 == 0)
			frlist[i].frptr = ulookup(hci->qh4) | 2;
		else if (i % 2 == 0)
			frlist[i].frptr = ulookup(hci->qh2) | 2;
		else
			frlist[i].frptr = ulookup(hci->qh1) | 2;
	}

	uhci_set_running(false);
	uhci_write(UhciRegs::FRBASE, ulookup(frlist));
}

void uhci_send(uhci_dev_extra_t* hci, u8 addr, u8 endp, bool ls, usb_request* request, void* response, u64 size, u64 mps) {
	uhci_qh_t* qh = uhci_alloc_qh();

	uhci_td_t* setup = uhci_alloc_td();
	vector<uhci_td_t*> data;
	// uhci_td_t* td2 = uhci_alloc_td();
	uhci_td_t* status = uhci_alloc_td();

	u32 num_data = align(size, mps) / mps;
	for (u32 i = 0; i < num_data; i++)
		data.push_back(uhci_alloc_td());
	
	if (num_data)
		setup->dword0.ptr = ulookup(data[0]);
	else
		setup->dword0.ptr = ulookup(status);
	setup->dword0.depthfirst = 1;

	status->dword0.t = 1;

	bool datatoggle = true;
	for (u32 i = 0; i < num_data; i++) {
		warn("assigning data stage with len %d", size ? (u32)(min(size, mps) - 1) : 0x7ff);

		u32 next;
		if (i == num_data - 1)
			next = ulookup(status);
		else
			next = ulookup(data[i + 1]);

		uhci_td_t* td2 = data[i];
		td2->dword0.ptr = next;
		td2->dword0.depthfirst = 1;
		td2->dword1 = {
			.actlen = 0,
			.reserved = 0,
			.sts = {
				.reserved = 0,
				.bitstuff_err = 0,
				.crc_or_timeout = 0,
				.nak = 0,
				.babble = 0,
				.databuf_err = 0,
				.stalled = 0,
				.active = 1,
			},
			.ioc = 0,
			.isochronous = 0,
			.ls = ls,
			.errcounter = 1,
			.spd = 0,
			.reserved2 = 0,
		};
		td2->dword2 = {
			.pid = UsbPid::IN, // IN  host szempontjából
			.addr = addr,
			.endp = endp,
			.data = datatoggle,
			.reserved = 0,
			.maxlen = size ? (u32)(min(size, mps) - 1) : 0x7ff,
		};
		td2->dword3 = ulookup(response) + i * mps;

		size -= mps;
		datatoggle = !datatoggle;
	}

	{
		setup->dword1 = {
			.actlen = 0,
			.reserved = 0,
			.sts = {
				.reserved = 0,
				.bitstuff_err = 0,
				.crc_or_timeout = 0,
				.nak = 0,
				.babble = 0,
				.databuf_err = 0,
				.stalled = 0,
				.active = 1,
			},
			.ioc = 0,
			.isochronous = 0,
			.ls = ls,
			.errcounter = 1,
			.spd = 0,
			.reserved2 = 0,
		};
		setup->dword2 = {
			.pid = UsbPid::SETUP,
			.addr = addr,
			.endp = endp,
			.data = 0,
			.reserved = 0,
			.maxlen = sizeof(usb_request) - 1,
		};
		setup->dword3 = ulookup(request);
	}

	{
		status->dword1 = {
			.actlen = 0,
			.reserved = 0,
			.sts = {
				.reserved = 0,
				.bitstuff_err = 0,
				.crc_or_timeout = 0,
				.nak = 0,
				.babble = 0,
				.databuf_err = 0,
				.stalled = 0,
				.active = 1,
			},
			.ioc = 1,
			.isochronous = 0,
			.ls = ls,
			.errcounter = 1,
			.spd = 0,
			.reserved2 = 0,
		};
		status->dword2 = {
			.pid = num_data ? UsbPid::OUT : UsbPid::IN,
			.addr = addr,
			.endp = endp,
			.data = 1,
			.reserved = 0,
			.maxlen = 0x7ff,
		};
		status->dword3 = 0;
	}

	qh->head.t = 1;
	qh->elem.elemptr = ulookup(setup);
	qh->elem.qh = 0;

	uhci_set_running(false);

	// Státusz lenullázása
	uhci_write(UhciRegs::USBSTS, 0xffff);

	hci->qhfs->elem.elemptr = ulookup<uhci_qh_t>(qh) | 2;

	uhci_set_running(true);
	arch_sleep(1);
	// running
	assert(uhci_read(UhciRegs::USBCMD) & 1);

	u64 time = arch_get_time();
	while ((uhci_read(UhciRegs::USBSTS) & 1) == 0) {
		if (arch_elapsed(time, 100)) {
			error("Timeout! Status: %02x %02x", setup->dword1.sts.raw, status->dword1.sts.raw);
			break;
		}
	}
	// printk("ms elapsed %lld\n", arch_get_time() - time);

	if (setup->dword1.sts.raw != 0)
		error("Setup status non-zero: %02x", setup->dword1.sts.raw);
	if (status->dword1.sts.raw != 0)
		error("Status packet status non-zero: %02x", status->dword1.sts.raw);
	for (u32 i = 0; i < num_data; i++)
		if (data[i]->dword1.sts.raw != 0)
			error("Data #%d status non-zero: %02x", i, data[i]->dword1.sts.raw);

	uhci_free(qh);
	uhci_free(setup);
	for (u32 i = 0; i < num_data; i++)
		uhci_free(data[i]);
	uhci_free(status);
}

bool uhci_init_port(uhci_dev_extra_t* hci, u8 portnum) {
	uhci_port_t port;
	uhci_send_reset(portnum);

	arch_sleep(20);

	port.raw = uhci_read(uhci_register(1));
	port.port_enable = 1;
	port.port_enable_ch = 1;
	uhci_write(uhci_register(portnum), port.raw);

	port.raw = uhci_read(uhci_register(portnum));
	if (!port.port_enable && port.dev_present)
		warn("Port %d has device but failed to be enabled", portnum);

	if (port.dev_present) {
		// Eszköz inicializálása
		// MPS méretű DEVICE desc., reset, set address, device descriptor de teljes méretbe
		device_t& dev = devmgr_add_device(device_t {
			.subsys = DevmgrSubsys::USB,
			.USB = {  }
		});

		u32 mps = port.ls ? 8 : 64;
		usb_request* devdesc = (usb_request*)uhci_alloc_qh();
		devdesc->bmRequestType = 0x80;
		devdesc->bRequest = UsbRequests::GET_DESCRIPTOR;
		devdesc->wValue = (1 << 8) | 0;
		devdesc->wIndex = 0;
		devdesc->wLength = mps;

		usb_descriptor_device* buf = (usb_descriptor_device*)pmm_alloc();

		uhci_send(hci, 0, 0, port.ls, devdesc, buf, 8, 8);
		uhci_send_reset(portnum);

		mps = buf->bMaxPacketSize;
		u32 addr = hci->addresses.find_and_set();
		report("Max Packet Size for device is %d; address to be assigned: %d", mps, addr);

		// uhci_send(hci, 0, 0, port.ls, devdesc, buf, 8, 8);
		// while (1);

		usb_request* setaddr = (usb_request*)uhci_alloc_qh();
		setaddr->bmRequestType = 0x00;
		setaddr->bRequest = UsbRequests::SET_ADDRESS;
		setaddr->wValue = addr;
		setaddr->wIndex = 0;
		setaddr->wLength = 0;

		arch_sleep(100);

		warn("sending setaddr");
		uhci_send(hci, 0, 0, port.ls, setaddr, nullptr, 0, 8);
		arch_sleep(2);

		warn("sent setaddr");

		devdesc->wLength = 18;

		uhci_send(hci, addr, 0, port.ls, devdesc, buf, 18, 8);

		warn("done: %04x:%04x", buf->idVendor, buf->idProduct);

		uhci_free(devdesc);
	}

	return port.dev_present;
}

extern "C" void mod_main(device_t* dev) {
	hciio = pci_read(*dev, PciRegs::BAR4);

	assert(hciio & 1);

	// Az alsó 2 bit metadata, le kell azokat szedni
	hciio &= ~0b11u;

	dev->PCI.extra = kmalloc(sizeof(uhci_dev_extra_t));
	uhci_dev_extra_t* hci = (uhci_dev_extra_t*)dev->PCI.extra;
	hci->addresses.init(kmalloc(128 / 8), 128);
	hci->addresses.set(0, true);

	u32 num_ports = uhci_num_ports();
	uhci_hc_init(dev);

	report("UHCI num ports: %d", num_ports);

	// Portok
	for (u32 i = 1; i <= num_ports; i++) {
		report("Port %d: device %s", i, uhci_init_port(hci, i) ? "present" : "not present");
	}

	pause();
}
