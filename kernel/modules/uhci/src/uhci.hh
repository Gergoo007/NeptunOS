#pragma once

#include <types.hh>

struct uhci_register_t {
	u16 io;
	u8 bytes;
};

struct UhciRegs {
	static constexpr uhci_register_t USBCMD		{ 0x00, 2 };
	static constexpr uhci_register_t USBSTS		{ 0x02, 2 };
	static constexpr uhci_register_t USBINTR	{ 0x04, 2 };
	static constexpr uhci_register_t FRNUM		{ 0x06, 2 };
	static constexpr uhci_register_t FRBASE		{ 0x08, 4 };
	static constexpr uhci_register_t SOFMOD		{ 0x0c, 1 };
	static constexpr uhci_register_t PORT1SC	{ 0x10, 2 };
	static constexpr uhci_register_t PORT2SC	{ 0x02, 2 };
};

struct uhci_usbcmd_t {
	u8 run : 1;
	u8 hcreset : 1;
	u8 globalreset : 1;
	u8 globalsuspend : 1;
	u8 globalresume : 1;
	u8 swdebug : 1;
	u8 configure : 1;
	u8 maxpktsize : 1; // 0: 32 bit a max packet size, 1: 64 bit
	u8 : 8;
};

struct uhci_usbsts_t {
	u16 intr : 1;
	u16 eintr : 1;
	u16 resume : 1;
	u16 syserr : 1;
	u16 procerr : 1;
	u16 halted : 1;
	u16 : 10;
};

struct uhci_usbintr_t {
	u16 timeout_crc : 1;
	u16 resume : 1;
	u16 complete_transfer : 1; // Küldjön-e a vezérlő megszakítást ha IOC-s TD végzett?
	u16 short_packet : 1; // Küldjön-e a vezérlő megszakítást ha short packet végzett?
	u16 : 10;
};


