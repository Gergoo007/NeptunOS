#include <devmgr/usb/usb.hh>
#include <devmgr/usb/hub.hh>
#include <devmgr/devmgr.hh>
#include <arch/amd64/paging.hh>
#include <arch/amd64/amd64.hh>

void* usb_alloc() {
	u64 a = (u64)pmm_alloc();
	assert(!(paging_lookup(a) >> 32));
	return (void*)a;
}

void usb_free(void* ptr) { pmm_free(ptr); }

const char* usb_get_string(device_t& usbdev, usb_descriptor_device* devdesc, u8 idx) {
	if (!idx) return "\0";
	auto* hciint = (usb_hci_interface_t*)usbdev.USB.hci->PCI.extra;

	usb_request* request = (usb_request*)usb_alloc();
	if (!usbdev.USB.langid) {
		// Van már LANGID?
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

		usb_free(langids);
	}

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

device_t& usb_device_add_skeleton(device_t& parent, u8 port, UsbSpeed speed) {
	u16 mps;
	switch (speed) {
		case UsbSpeed::LS:
			mps = 8;
			break;
		case UsbSpeed::FS:
		case UsbSpeed::HS:
			mps = 64;
			break;
		case UsbSpeed::SS:
			mps = 512;
			break;
	}

	return devmgr_add_device(device_t {
		.parent = &parent,
		.subsys = DevmgrSubsys::USB,
		.USB = {
			.vendor = 0,
			.product = 0,
			// USB Hub?
			.hci = (parent.subsys == DevmgrSubsys::USB) ? parent.USB.hci : &parent,
			.manufacturerName = nullptr,
			.productName = nullptr,
			.serial = nullptr,
			.mps = mps,
			.langid = (u16)-1,
			.addr = 0,
			.portnum = port,
			.class_ = 0,
			.subclass = 0,
			.progif = 0,
			.speed = speed,
		},
	});
}

void usb_send_reset(device_t& usbdev) {
	auto hciint = ((usb_hci_interface_t*)(usbdev.USB.hci->PCI.extra));
	if (usbdev.parent != usbdev.USB.hci) {
		debug("Device to be reset is on a hub...");
		auto& hub = *usbdev.parent;
		assert(hub.subsys == DevmgrSubsys::USB);

		usb_hub_send_reset(usbdev, usbdev.USB.portnum);
	} else {
		debug("Device to be reset is directly on the root hub...");
		hciint->usb_reset_port(usbdev);
	}
}

// Kiegészíti a DEVICE leírót (ha MSP < 8), ad egy címet az eszköznek, és beállít egy konfigot
void usb_init(device_t& usbdev) {
	usb_request* request = (usb_request*)usb_alloc();
	request->bmRequestType = 0x80;
	request->bRequest = UsbRequests::GET_DESCRIPTOR;
	request->wValue = (1 << 8) | 0;
	request->wIndex = 0;
	request->wLength = usbdev.USB.mps;
	if (request->wLength < 18)
		fatal("TODO: mps(=%d) < 18", usbdev.USB.mps);

	usb_descriptor_device* devdesc = (usb_descriptor_device*)((u64)request + 64);
	memset(devdesc, 0, sizeof(*devdesc));

	auto hciint = ((usb_hci_interface_t*)(usbdev.USB.hci->PCI.extra));
	debug("Sending GET_DESCRIPTOR DEVICE request #1...");
	hciint->usb_send(usbdev, 0, request, devdesc);

	usbdev.USB.mps = devdesc->bMaxPacketSize;
	usbdev.USB.classcode = devdesc->classcode;
	usbdev.USB.vendor = devdesc->idVendor;
	usbdev.USB.product = devdesc->idProduct;
	
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

	request->bmRequestType = 0x80;
	request->bRequest = UsbRequests::GET_DESCRIPTOR;
	request->wValue = (1 << 8) | 0;
	request->wIndex = 0;
	request->wLength = 18;

	usbdev.USB.manufacturerName = usb_get_string(usbdev, devdesc, devdesc->iManufacturer);
	usbdev.USB.productName = usb_get_string(usbdev, devdesc, devdesc->iProduct);
	usbdev.USB.serial = usb_get_string(usbdev, devdesc, devdesc->iSerialNumber);

	if (devdesc->iProduct || devdesc->iManufacturer)
		debug("USB device read: %s %s %s", usbdev.USB.manufacturerName, usbdev.USB.productName, usbdev.USB.serial);

	// Select config
	request->bmRequestType = 0x80;
	request->bRequest = UsbRequests::GET_DESCRIPTOR;
	request->wValueL = 0;
	request->wValueH = 2;
	request->wIndex = 0;
	request->wLength = 9;
	usb_descriptor_configuration* config = (usb_descriptor_configuration*)((u64)devdesc + 64);

	u8 preferredConfig = 0;
	for (u32 i = 0; i < devdesc->bNumConfigurations; i++) {
		request->wValueL = i;
		hciint->usb_send(usbdev, 0, request, config);
		
		const char* s = usb_get_string(usbdev, devdesc, config->iConfiguration);
		debug("config \"%s\": %d mA; %d interfaces; ", s, config->bMaxPower * 2, config->bNumInterfaces);
		preferredConfig = config->bConfigurationValue;
	}
	
	request->bmRequestType = 0x00;
	request->bRequest = UsbRequests::SET_CONFIGURATION;
	request->wValue = preferredConfig;
	request->wIndex = 0;
	request->wLength = 0;
	hciint->usb_send(usbdev, 0, request, nullptr);

	switch (devdesc->bDeviceClass) {
		case UsbClass::HUB:
			if (!devdesc->bDeviceSubClass)
				usb_hub_init(usbdev);
			break;

		default: break;
	}

	usb_free(request);
}

void usb_init_all() {
	// I need to store the original size because usb_hub_init appends this vector
	// Elements are guaranteed to be appended to the back of the vec, which means
	// origsize only contains elements not appended by usb_init
	const u32 origsize = devices.size;
	for (u32 i = 0; i < origsize; i++) {
		if (devices[i]->subsys != DevmgrSubsys::USB)
			continue;

		usb_init(*devices[i]);
	}
}
