#pragma once

#include <devmgr/usb/usb.hh>

void usb_hub_init(Device& usbdev);
UsbSpeed usb_hub_send_reset(Device& usbdev, u8 portnum);
