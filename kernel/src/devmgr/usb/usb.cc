#include <devmgr/usb/usb.hh>
#include <devmgr/usb/hub.hh>
#include <devmgr/usb/hid.hh>
#include <devmgr/devmgr.hh>
#include <arch/amd64/paging.hh>
#include <arch/amd64/amd64.hh>
#include <mm/pmm4g.hh>

const char* usb_get_string(device_t& usbdev, u8 idx) {
	if (!idx) return "";
	auto* hciint = (usb_hci_interface_t*)usbdev.kinds.get<device_t_USB>().hci->extra;

	usb_request* request = (usb_request*)kmalloc4g(sizeof(usb_request));
	if (usbdev.kinds.get<device_t_USB>().langid == (u16)-1) {
		// Van már LANGID?
		usb_descriptor_string_langids* langids = (usb_descriptor_string_langids*)kmalloc4g(sizeof(usb_descriptor_string_langids));

		request->bmRequestType = 0x80;
		request->bRequest = UsbRequests::GET_DESCRIPTOR;
		request->wValueL = 0;
		request->wValueH = 3;
		request->wIndex = 0;
		request->wLength = 2;
		
		hciint->usb_send(usbdev, 0, request, langids);
		u32 num_langids = (langids->hdr.bLength - 2) / 2;
		langids = (usb_descriptor_string_langids*)krealloc4g(langids, langids->hdr.bLength);
		request->wLength = 2 + num_langids * 2;
		hciint->usb_send(usbdev, 0, request, langids);

		// Default is US English
		usbdev.kinds.get<device_t_USB>().langid = num_langids ? langids->wLangID[0] : 0x0409;
		for (u32 i = 0; i < num_langids; i++) {
			debug("Device supports LANGID %04x", langids->wLangID[i]);
			if (langids->wLangID[i] == 0x040e) // Hunagrian
				usbdev.kinds.get<device_t_USB>().langid = langids->wLangID[i];
		}

		kfree4g(langids);
	}

	request->bmRequestType = 0x80;
	request->bRequest = UsbRequests::GET_DESCRIPTOR;
	request->wIndex = usbdev.kinds.get<device_t_USB>().langid;
	request->wValueL = idx;
	request->wValueH = 3;
	// Először csak a bLength kell
	request->wLength = 2;

	usb_descriptor_string* string = (usb_descriptor_string*)kmalloc4g(256);
	request->wIndex = usbdev.kinds.get<device_t_USB>().langid;
	request->wValueL = idx;
	request->bmRequestType = 0x80;
	// Először csak a bLength kell
	request->wLength = 2;
	hciint->usb_send(usbdev, 0, request, string);

	request->wLength = string->hdr.bLength;
	// string = (usb_descriptor_string*)krealloc4g(string, string->hdr.bLength);
	hciint->usb_send(usbdev, 0, request, string);

	u32 len = (string->hdr.bLength - 2) / 2;
	char* str = (char*)kmalloc(len + 1);
	str[len] = 0;
	ucs2_to_asciin(string->string, str, len);
	vmm_check(str);

	kfree4g(request);
	kfree4g(string);

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
		.extra = nullptr,
		.subsys = DevmgrSubsys::USB,
		.loc = port,
		.kinds = devunion_t(true, device_t_USB {
			.vendor = 0,
			.product = 0,
			// USB Hub?
			.hci = (parent.subsys == DevmgrSubsys::USB) ? parent.kinds.get<device_t_USB>().hci : &parent,
			.manufacturerName = string(),
			.productName = string(),
			.serial = string(),
			.mps = mps,
			.langid = (u16)-1,
			.addr = 0,
			.class_ = 0,
			.subclass = 0,
			.progif = 0,
			.speed = speed,
		})
	});
}

void usb_send_reset(device_t& usbdev) {
	auto hciint = ((usb_hci_interface_t*)(usbdev.kinds.get<device_t_USB>().hci->extra));
	if (usbdev.parent != usbdev.kinds.get<device_t_USB>().hci) {
		debug("Device to be reset is on a hub...");
		auto& hub = *usbdev.parent;
		assert(hub.subsys == DevmgrSubsys::USB);

		usb_hub_send_reset(hub, usbdev.loc);
	} else {
		debug("Device to be reset is directly on the root hub...");
		hciint->usb_reset_port(usbdev);
	}
}

void usb_send(device_t& usbdev, u8 endp, usb_request* req, void* data) {
	((usb_hci_interface_t*)(usbdev.kinds.get<device_t_USB>().hci->extra))->usb_send(usbdev, endp, req, data);
	arch_sleep(2, true);
}

// Kiegészíti a DEVICE leírót (ha MSP < 8), ad egy címet az eszköznek, és beállít egy konfigot
void usb_init(device_t& usbdev) {
	usb_request* request = (usb_request*)kmalloc4g(sizeof(usb_request));
	request->bmRequestType = 0x80;
	request->bRequest = UsbRequests::GET_DESCRIPTOR;
	request->wValueH = 1;
	request->wValueL = 0;
	request->wIndex = 0;
	request->wLength = usbdev.kinds.get<device_t_USB>().mps;
	if (request->wLength < 18)
		fatal("TODO: mps(=%d) < 18", usbdev.kinds.get<device_t_USB>().mps);

	usb_descriptor_device* devdesc = (usb_descriptor_device*)kmalloc4g(sizeof(usb_descriptor_device));
	memset(devdesc, 0, sizeof(*devdesc));

	auto hciint = ((usb_hci_interface_t*)(usbdev.kinds.get<device_t_USB>().hci->extra));
	debug("Sending GET_DESCRIPTOR DEVICE request #1...");
	hciint->usb_send(usbdev, 0, request, devdesc);

	usb_send_reset(usbdev);

	usbdev.kinds.get<device_t_USB>().mps = devdesc->bMaxPacketSize;
	usbdev.kinds.get<device_t_USB>().classcode = devdesc->classcode;
	usbdev.kinds.get<device_t_USB>().vendor = devdesc->idVendor;
	usbdev.kinds.get<device_t_USB>().product = devdesc->idProduct;
	
	if (!(devdesc->bMaxPacketSize == 8 || devdesc->bMaxPacketSize == 16 || devdesc->bMaxPacketSize == 32 || devdesc->bMaxPacketSize == 64))
		fatal("Invalid MPS: %d", devdesc->bMaxPacketSize);
	// else
	// 	debug("Device MPS is %d", devdesc->bMaxPacketSize);
	u32 addr = hciint->usb_make_address(*usbdev.kinds.get<device_t_USB>().hci);

	request->bmRequestType = 0x00;
	request->bRequest = UsbRequests::SET_ADDRESS;
	request->wValue = addr;
	request->wIndex = 0;
	request->wLength = 0;

	debug("Sending SET_ADDRESS request...");
	hciint->usb_send(usbdev, 0, request, nullptr);

	usbdev.kinds.get<device_t_USB>().addr = addr;

	arch_sleep(10, true);

	request->bmRequestType = 0x80;
	request->bRequest = UsbRequests::GET_DESCRIPTOR;
	request->wValueH = 1;
	request->wValueL = 0;
	request->wIndex = 0;
	request->wLength = 18;
	debug("Sending GET_DESCRIPTOR DEVICE request #2...");
	hciint->usb_send(usbdev, 0, request, devdesc);

	usbdev.kinds.get<device_t_USB>().manufacturerName = string(usb_get_string(usbdev, devdesc->iManufacturer));
	usbdev.kinds.get<device_t_USB>().productName = string(usb_get_string(usbdev, devdesc->iProduct));
	usbdev.kinds.get<device_t_USB>().serial = string(usb_get_string(usbdev, devdesc->iSerialNumber));

	if (devdesc->iProduct || devdesc->iManufacturer)
		debug("USB %d device %04x:%04x read: %s %s %s", (u32)usbdev.kinds.get<device_t_USB>().speed, usbdev.kinds.get<device_t_USB>().vendor, usbdev.kinds.get<device_t_USB>().product, usbdev.kinds.get<device_t_USB>().manufacturerName.c_str(), usbdev.kinds.get<device_t_USB>().productName.c_str(), usbdev.kinds.get<device_t_USB>().serial.c_str());
	else
		debug("USB device without strings: %04x:%04x", devdesc->idVendor, devdesc->idProduct);

	// Select config
	request->bmRequestType = 0x80;
	request->bRequest = UsbRequests::GET_DESCRIPTOR;
	request->wValueL = 0;
	request->wValueH = 2;
	request->wIndex = 0;
	request->wLength = 9;
	usb_descriptor_configuration* config = (usb_descriptor_configuration*)kmalloc4g(64);

	u8 preferredConfig = 0;
	for (u32 i = 0; i < devdesc->bNumConfigurations; i++) {
		request->wValueL = i;

		// wTotalLength kell csak először, majd minden más
		// request->wLength = 4;
		// hciint->usb_send(usbdev, 0, request, config);
		hciint->usb_send(usbdev, 0, request, config);
		request->wLength = config->wTotalLength;
		config = (usb_descriptor_configuration*)krealloc4g(config, config->wTotalLength);
		hciint->usb_send(usbdev, 0, request, config);
		
		// i16 len = config->wTotalLength;
		const char* s = "";
		if (devdesc->idVendor == 0x0a5c && devdesc->idProduct == 0x217f) {
			// TODO: Ezen a szaron nem lehet ezt a string descriptort lekérni
		} else {
			s = usb_get_string(usbdev, config->iConfiguration);
		}
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

		case UsbClass::HID:
			usb_hid_init(usbdev);
			break;

		default: break;
	}

	kfree4g(request);
	kfree4g(devdesc);
	kfree4g(config);
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
