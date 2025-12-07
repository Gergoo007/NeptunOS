#include <devmgr/usb/usb.hh>
#include <devmgr/devmgr.hh>
#include <arch/amd64/paging.hh>
#include <arch/amd64/amd64.hh>

void* usb_alloc() {
	u64 a = (u64)pmm_alloc();
	assert(!(paging_lookup(a) >> 32));
	return (void*)a;
}

void usb_free(void* ptr) { pmm_free(ptr); }

char* usb_get_string(device_t& usbdev, usb_descriptor_device* devdesc, u8 idx) {
	auto* hciint = (usb_hci_interface_t*)usbdev.USB.hci->PCI.extra;

	usb_request* request = (usb_request*)usb_alloc();
	request->bmRequestType = 0x80;
	request->bRequest = UsbRequests::GET_DESCRIPTOR;
	request->wIndex = usbdev.USB.langid;
	request->wValueH = 3;
	request->wValueL = idx;
	// Először csak a bLength kell
	request->wLength = 2;

	usb_descriptor_string* string = (usb_descriptor_string*)usb_alloc();
	request->wIndex = usbdev.USB.langid;
	request->wValueL = idx;
	request->bmRequestType = 0x80;
	// Először csak a bLength kell
	request->wLength = 2;
	hciint->usb_send(usbdev, 0, request, string);
	request->wLength = string->hdr.bLength;
	hciint->usb_send(usbdev, 0, request, string);

	u32 len = (string->hdr.bLength - 2) / 2;
	char* str = (char*)kmalloc(len / 2 + 1);
	str[len] = 0;
	ucs2_to_asciin(string->string, str, len);

	usb_free(request);
	usb_free(string);

	return str;
}

// Kiegészíti a DEVICE leírót (ha MSP < 8), ad egy címet az eszköznek
void usb_init(device_t& usbdev) {
	usb_request* request = (usb_request*)usb_alloc();
	request->bmRequestType = 0x80;
	request->bRequest = UsbRequests::GET_DESCRIPTOR;
	request->wValue = (1 << 8) | 0;
	request->wIndex = 0;
	request->wLength = usbdev.USB.mps;

	usb_descriptor_device* devdesc = (usb_descriptor_device*)(request + 64);
	memset(devdesc, 0, sizeof(*devdesc));

	auto hciint = ((usb_hci_interface_t*)(usbdev.USB.hci->PCI.extra));
	debug("Sending GET_DESCRIPTOR DEVICE request #1...");
	hciint->usb_send(usbdev, 0, request, devdesc);
	usbdev.USB.mps = devdesc->bMaxPacketSize;
	if (!(devdesc->bMaxPacketSize == 8 || devdesc->bMaxPacketSize == 16 || devdesc->bMaxPacketSize == 32 || devdesc->bMaxPacketSize == 64))
		fatal("Invalid MPS: %d", devdesc->bMaxPacketSize);
	u32 addr = hciint->usb_make_address(*usbdev.USB.hci);

	request->bmRequestType = 0x00;
	request->bRequest = UsbRequests::SET_ADDRESS;
	request->wValue = addr;
	request->wIndex = 0;
	request->wLength = 0;

	debug("Sending SET_ADDRESS request...");
	hciint->usb_send(usbdev, 0, request, nullptr);

	usbdev.USB.addr = addr;

	arch_sleep(10);

	usb_descriptor_string_langids* langids = (usb_descriptor_string_langids*)usb_alloc();

	request->bmRequestType = 0x80;
	request->bRequest = UsbRequests::GET_DESCRIPTOR;
	request->wValueL = 0;
	request->wValueH = 3;
	request->wIndex = 0;
	request->wLength = 2;

	hciint->usb_send(usbdev, 0, request, langids);
	u32 num_langids = (langids->hdr.bLength - 2) / 2;
	request->wLength = 2 + num_langids * 2;
	hciint->usb_send(usbdev, 0, request, langids);

	// Default is US English
	usbdev.USB.langid = num_langids ? langids->wLangID[0] : 0x0409;
	for (u32 i = 0; i < num_langids; i++) {
		debug("Device supports LANGID %04x", langids->wLangID[i]);
		if (langids->wLangID[i] == 0x040e) // Hunagrian
			usbdev.USB.langid = langids->wLangID[i];
	}

	request->bmRequestType = 0x80;
	request->bRequest = UsbRequests::GET_DESCRIPTOR;
	request->wValue = (1 << 8) | 0;
	request->wIndex = 0;
	request->wLength = 18;

	debug("Sending GET_DESCRIPTOR DEVICE request #2...");
	hciint->usb_send(usbdev, 0, request, devdesc);

	if (!devdesc->iManufacturer && !devdesc->iProduct && !devdesc->iSerialNumber) {
		warn("Device %04x:%04x does not support STRING descriptors!", devdesc->idVendor, devdesc->idProduct);
	} else {
		debug("Sending GET_DESCRIPTOR STRING requests...");
		
		usbdev.USB.manufacturerName = usb_get_string(usbdev, devdesc, devdesc->iManufacturer);
		usbdev.USB.productName = usb_get_string(usbdev, devdesc, devdesc->iProduct);
		usbdev.USB.serial = usb_get_string(usbdev, devdesc, devdesc->iSerialNumber);
	}

	warn("Device initialized: %04x:%04x", devdesc->idVendor, devdesc->idProduct);

	usb_free(request);
	usb_free(langids);
}

void usb_init_all() {
	for (auto& d : devices) {
		if (d->subsys != DevmgrSubsys::USB)
			continue;

		usb_init(*d);
	}
}
