#include <devmgr/usb/hub.hh>
#include <arch/amd64/amd64.hh>
#include <mm/pmm4g.hh>

UsbSpeed usb_hub_send_reset(Device& hub, u8 port) {
	usb_request* getsts = (usb_request*)kmalloc4g(sizeof(usb_request));
	getsts->bmRequestType = 0b10100011;
	getsts->bRequest = UsbRequests::GET_STATUS;
	getsts->wIndex = port;
	getsts->wValue = 0;
	getsts->wLength = 4;

	usb_request* setfeat = (usb_request*)kmalloc4g(sizeof(usb_request));
	setfeat->bmRequestType = 0b00100011;
	setfeat->bRequest = UsbRequests::SET_FEATURE;
	setfeat->wIndex = port;
	setfeat->wValue = 4; // PORT_RESET
	setfeat->wLength = 0;

	auto hciint = ((usb_hci_interface_t*)(hub.kinds.get<device_t_USB>().hci->extra));
	hciint->usb_send(hub, 0, setfeat, nullptr);

	arch_sleep(50, true);

	usb_hub_port_feats* portsc = (usb_hub_port_feats*)kmalloc4g(sizeof(usb_hub_port_feats));

	// If the port is not reset even after 6 tries, abandon it
	bool success = false;
	UsbSpeed speed;
	for (u32 j = 0; j < 5; j++) {
		arch_sleep(20, true);
		hciint->usb_send(hub, 0, getsts, portsc);
		if (portsc->PORT_ENABLE) {
			if (portsc->PORT_LOW_SPEED)
				speed = UsbSpeed::LS;
			else if (portsc->PORT_HIGH_SPEED)
				speed = UsbSpeed::HS;
			else
				speed = UsbSpeed::FS;
			debug("Port %d functional after %d tries! Speed: %d", port, j, speed);
			success = true;
			break;
		}
	}

	if (!success)
		fatal("Port failed to enable! Port status: %06x", portsc->raw);

	kfree4g(getsts);
	kfree4g(setfeat);
	kfree4g(portsc);

	return speed;
}

void usb_hub_init(Device &usbdev) {
	debug("Initializing USB Hub with progif %02x...", usbdev.kinds.get<device_t_USB>().progif);

	auto hciint = ((usb_hci_interface_t*)(usbdev.kinds.get<device_t_USB>().hci->extra));
	usb_request* r = (usb_request*)kmalloc4g(sizeof(usb_request));
	r->bmRequestType = 0b10100000;
	r->bRequest = UsbRequests::GET_DESCRIPTOR;
	r->wValueH = 0x29;
	r->wValueL = 0x00;
	r->wIndex = 0;
	r->wLength = 2;

	usb_descriptor_hub* hub = (usb_descriptor_hub*)kmalloc4g(sizeof(usb_descriptor_hub));
	hciint->usb_send(usbdev, 0, r, hub);
	r->wLength = hub->hdr.bLength;
	hciint->usb_send(usbdev, 0, r, hub);

	debug("num ports %d", hub->bNbrPorts);

	usb_hub_port_feats* portsc = (usb_hub_port_feats*)kmalloc4g(sizeof(usb_hub_port_feats));

	// Port
	// Send power-up
	r->bmRequestType = 0x23;
	r->bRequest = UsbRequests::SET_FEATURE;
	r->wValue = 8;
	r->wLength = 0;

	for (u32 i = 1; i <= hub->bNbrPorts; i++) {
		r->wIndex = i; // port
		hciint->usb_send(usbdev, 0, r, nullptr);
	}

	arch_sleep(hub->bPowerOnGood*2, true);

	// usb_request* setfeat = (usb_request*)((u64)r + 192);
	// setfeat->bmRequestType = 0b00100011;
	// setfeat->bRequest = UsbRequests::SET_FEATURE;
	// setfeat->wIndex = 0;
	// setfeat->wValue = 4; // PORT_RESET
	// setfeat->wLength = 0;

	usb_request* getsts = (usb_request*)kmalloc4g(sizeof(usb_request));
	getsts->bmRequestType = 0xa3;
	getsts->bRequest = UsbRequests::GET_STATUS;
	getsts->wIndex = 0;
	getsts->wValue = 0;
	getsts->wLength = 4;

	for (u32 i = 1; i <= hub->bNbrPorts; i++) {
		getsts->wIndex = i; // port
		hciint->usb_send(usbdev, 0, getsts, portsc);
		if (portsc->PORT_CONNECTION) {
			auto speed = usb_hub_send_reset(usbdev, i);
			
			// The reset was prolly successful
			auto& d = usb_device_add_skeleton(usbdev, i, speed);
			debug("Starting init of hub device on port %d", i);
			usb_init(d);
		}
	}

	kfree4g(r);
	kfree4g(hub);
	kfree4g(portsc);
	kfree4g(getsts);
}
