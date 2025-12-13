#pragma once

#include <devmgr/usb/usb.hh>

void usb_hub_init(device_t& usbdev);
UsbSpeed usb_hub_send_reset(device_t& usbdev, u8 portnum);
