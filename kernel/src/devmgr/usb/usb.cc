#include <devmgr/usb/usb.hh>
#include <devmgr/devmgr.hh>
#include <arch/amd64/paging.hh>

void* usb_alloc() {
	u64 a = (u64)pmm_alloc();
	if constexpr(debug) assert(!(paging_lookup(a) >> 32));
	return PHYSICAL((void*)a);
}

// Elküldi az inicializáló parancsokat a 0-ás címre
void usb_init(device_t& usbdev) {
	usb_request* devdesc = (usb_request*)usb_alloc();
	devdesc->bmRequestType = 0x80;
	devdesc->bRequest = UsbRequests::GET_DESCRIPTOR;
	devdesc->wValue = (1 << 8) | 0;
	devdesc->wIndex = 0;
	devdesc->wLength = usbdev.USB.mps;

	usb_descriptor_device* buf = (usb_descriptor_device*)pmm_alloc();

	auto hciint = ((usb_hci_interface_t*)(usbdev.USB.hci->PCI.extra));
	hciint->usb_send(usbdev, 0, 0, devdesc, buf, 8);

	usbdev.USB.mps = buf->bMaxPacketSize;
	u32 addr = hciint->usb_make_address(*usbdev.USB.hci);
	// report("Max Packet Size for device is %d; address to be assigned: %d", mps, addr);

	// // uhci_send(hci, 0, 0, port.ls, devdesc, buf, 8, 8);
	// // while (1);

	// usb_request* setaddr = (usb_request*)usb_alloc();
	// setaddr->bmRequestType = 0x00;
	// setaddr->bRequest = UsbRequests::SET_ADDRESS;
	// setaddr->wValue = addr;
	// setaddr->wIndex = 0;
	// setaddr->wLength = 0;

	// arch_sleep(100);

	// warn("sending setaddr");
	// uhci_send(hci, 0, 0, port.ls, setaddr, nullptr, 0, 8);
	// arch_sleep(2);

	// warn("sent setaddr");

	// devdesc->wLength = 18;

	// uhci_send(hci, addr, 0, port.ls, devdesc, buf, 18, 8);

	// warn("done: %04x:%04x", buf->idVendor, buf->idProduct);

	// uhci_free(devdesc);
}

void usb_init_all() {
	// for (auto& d : devices) {
	// 	if (d.subsys != DevmgrSubsys::USB)
	// 		continue;

	// 	usb_init(d);
	// }
}
