#pragma once

#include <types.hh>
#include <util/bitmap.hh>
#include <devmgr/usb/usb.hh>
#include <devmgr/devmgr.hh>

struct uhci_register {
	u16 io;
	u8 bytes;

	constexpr uhci_register(u16 io, u8 bytes): io(io), bytes(bytes) {  }
	constexpr uhci_register(u8 portnum): io(0x10 + (portnum - 1) * 2), bytes(2) {  }
};

struct UhciRegs {
	static constexpr uhci_register USBCMD		{ 0x00, 2 };
	static constexpr uhci_register USBSTS		{ 0x02, 2 };
	static constexpr uhci_register USBINTR		{ 0x04, 2 };
	static constexpr uhci_register FRNUM		{ 0x06, 2 };
	static constexpr uhci_register FRBASE		{ 0x08, 4 };
	static constexpr uhci_register SOFMOD		{ 0x0c, 1 };
};

punion uhci_usbcmd_t {
	pstruct {
		u8 run : 1;
		u8 hcreset : 1;
		u8 globalreset : 1;
		u8 globalsuspend : 1;
		u8 globalresume : 1;
		u8 swdebug : 1;
		u8 configure : 1;
		u8 maxpktsize : 1; // 0: 32 bit a max packet size, 1: 64 bit
		u8 reserved : 8;
	};
	u16 raw;
};

punion uhci_usbsts_t {
	pstruct {
		u16 intr : 1;
		u16 eintr : 1;
		u16 resume : 1;
		u16 syserr : 1;
		u16 procerr : 1;
		u16 halted : 1;
		u16 reserved : 10;
	};
	u16 raw;
};

punion uhci_usbintr_t {
	pstruct {
		u16 timeout_crc : 1;
		u16 resume : 1;
		u16 complete_transfer : 1; // Küldjön-e a vezérlő megszakítást ha IOC-s TD végzett?
		u16 short_packet : 1; // Küldjön-e a vezérlő megszakítást ha short packet végzett?
		u16 reserved : 10;
	};
	u16 raw;
};

pstruct uhci_td_t {
	punion {
		pstruct {
			u32 t : 1;
			u32 qh : 1;
			u32 depthfirst : 1;
			u32 reserved : 1;
		};
		u32 ptr;
	} dword0;
	pstruct {
		u32 actlen : 11;
		u32 reserved : 5;
		// u32 sts : 8;
		punion {
			pstruct {
				u8 reserved : 1;
				u8 bitstuff_err : 1;
				u8 crc_or_timeout : 1;
				u8 nak : 1;
				u8 babble : 1;
				u8 databuf_err : 1;
				u8 stalled : 1;
				u8 active : 1;
			};
			u8 raw;
		} sts;
		u32 ioc : 1;
		u32 isochronous : 1;
		u32 ls : 1;
		u32 errcounter : 2; // nem-nullára be kell majd állítani
		// ha ez true, és kevesebb adat jött át mint a Max Packet Hossz, küld egy interruptot a vezérlő és
		// inaktívnak jelöli a TD-t
		u32 spd : 1;
		u32 reserved2 : 2;
	} dword1;
	pstruct {
		u32 pid : 8;
		u32 addr : 7;
		u32 endp : 4;
		u32 data : 1;
		u32 reserved : 1;
		u32 maxlen : 11; // ez igazából eggyel kisebb mint a tényleges érték
	} dword2;
	u32 dword3; // buffer

	// programozó azt csinál ezekkel amit akar
	u32 dword4;
	u32 dword5;
	u32 dword6;
	u32 dword7;
};

pstruct uhci_qh_t {
	punion {
		pstruct {
			u32 t : 1;
			u32 qh : 1;
			u32 reserved : 2;
		};
		u32 headptr;
	} head;
	punion {
		pstruct {
			u32 t : 1;
			u32 qh : 1;
			u32 reserved : 2;
		};
		u32 elemptr;
	} elem;
};

punion uhci_frame_t {
	pstruct {
		u32 t : 1;
		u32 qh : 1;
		u32 reserved : 2;
	};
	u32 frptr;
};

punion uhci_port_t {
	pstruct {
		u16 dev_present : 1;
		u16 dev_present_ch : 1;
		u16 port_enable : 1;
		u16 port_enable_ch : 1;
		u16 linests : 2;
		u16 resume_detected : 1;
		u16 : 1;
		u16 ls : 1;
		u16 reset : 1;
		u16 : 2;
		u16 suspend : 1;
		u16 : 3;
	};
	u16 raw;
};

struct uhci_dev_extra_t : usb_hci_interface_t {
	// usb_hci_interface_t interface;

	uhci_qh_t* qh1;
	uhci_qh_t* qh2;
	uhci_qh_t* qh4;
	uhci_qh_t* qh8;
	uhci_qh_t* qh16;
	uhci_qh_t* qh32;
	uhci_qh_t* qh64;
	uhci_qh_t* qh128;
	uhci_qh_t* qhfs;
	uhci_qh_t* qhls;

	bitmap_t addresses;

	u16 hciio;
};

struct uhci_internal {
	uhci_dev_extra_t* extra;
	device_t& hci;
};
