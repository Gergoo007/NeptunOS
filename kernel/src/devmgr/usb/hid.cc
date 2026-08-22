#include <devmgr/usb/hid.hh>
#include <mm/pmm4g.hh>

void usb_hid_init(Device& usbdev, u8 firstendp) {
	if (usbdev.kinds.get<device_t_USB>().subclass == 0x01)
		warn("This USB HID supports the Boot protocol!");
	else
		warn("This USB HID does not support the Boot Protocol!");

	usb_request* r = (usb_request*)kmalloc4g(sizeof(usb_request));
	r->bmRequestType = 0b10000001;
	r->bRequest = UsbRequests::GET_DESCRIPTOR;
	r->wValueH = UsbDescriptors::REPORT;
	r->wValueL = 0;
	r->wIndex = 0; // TODO: interface number
	r->wLength = 2;

	pause();
}
