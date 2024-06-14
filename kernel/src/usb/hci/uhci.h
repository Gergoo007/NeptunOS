#pragma once

#include <usb/usb.h>

#include <apic/apic.h>
#include <arch/x86/idt.h>
#include <pci/pci.h>
#include <serial/com.h>
#include <apic/pit/pit.h>

#pragma pack(1)

// Credit for definitions below: https://github.com/pdoane/osdev/

enum {
	PORT_CONN = 	(1 << 0),
	PORT_C_CHANGE =	(1 << 1),
	PORT_ENABLED = 	(1 << 2),
	PORT_E_CHANGE =	(1 << 3),
	PORT_LINE_STS =	(1 << 4),
	PORT_RESUME =	(1 << 6),
	PORT_LOWSP =	(1 << 8),
	PORT_DEV_RESET =(1 << 9),
	PORT_SUSPEND =	(1 << 12),
	PORT_RWC =		(PORT_C_CHANGE | PORT_E_CHANGE),
};

enum {
	REG_CMD			= 0x00, // USB Command
	REG_STS			= 0x02, // USB Status
	REG_INTR		= 0x04, // USB Interrupt Enable
	REG_FRNUM		= 0x06, // Frame Number
	REG_FRBASEADD	= 0x08, // Frame List Base Address
	REG_SOFMOD		= 0x0C, // Start of Frame Modify
	REG_PORT1		= 0x10, // Port 1 Status/Control
	REG_PORT2		= 0x12, // Port 2 Status/Control
	REG_LEGSUP		= 0xc0, // Legacy Support
};

enum {
	CMD_RS		= (1 << 0), // Run/Stop
	CMD_HCRESET = (1 << 1), // Host Controller Reset
	CMD_GRESET	= (1 << 2), // Global Reset
	CMD_EGSM	= (1 << 3), // Enter Global Suspend Resume
	CMD_FGR		= (1 << 4), // Force Global Resume
	CMD_SWDBG 	= (1 << 5), // Software Debug
	CMD_CF		= (1 << 6), // Configure Flag
	CMD_MAXP	= (1 << 7), // Max Packet (0 = 32, 1 = 64)
};

enum {
	TD_LPTR_TERM	= (1 << 0),
	TD_LPTR_QH 		= (1 << 1),
	TD_LPTR_DEPTH	= (1 << 2),
};

enum {
	TD_CS_ACTLEN		= 0x07ff,
	TD_CS_BITSTUFF		= (1 << 17), // Bitstuff Error
	TD_CS_CRC_TIMEOUT	= (1 << 18), // CRC/Time Out Error
	TD_CS_NAK			= (1 << 19), // NAK Received
	TD_CS_BABBLE		= (1 << 20), // Babble Detected
	TD_CS_DATABUF	 	= (1 << 21), // Data Buffer Error
	TD_CS_STALL			= (1 << 22), // Stalled
	TD_CS_ACTIVE		= (1 << 23), // Active
	TD_CS_INT_ON_C		= (1 << 24), // Interrupt on Complete
	TD_CS_ISOCHR_SEL 	= (1 << 25), // Isochronous Select
	TD_CS_LS			= (1 << 26), // Low Speed Device
	TD_CS_ERR_MASK 		= (3 << 27), // Error counter
	TD_CS_ERR_SHIFT		= 0x1b,
	TD_CS_SPD			= (1 << 29), // Short Packet Detect
};

enum {
	TD_TOK_PID_MASK			= 0x00ff,	  // Packet Identification
	TD_TOK_DEVADDR_MASK 	= 0x7f00,	  // Device Address
	TD_TOK_DEVADDR_SHIFT	= 8,
	TD_TOK_ENDP_MASK		= 0x00078000, // Endpoint
	TD_TOK_ENDP_SHIFT 		= 15,
	TD_TOK_DATA_MASK		= 0x00080000, // Data Toggle
	TD_TOK_DATA_SHIFT		= 19,
	TD_TOK_MAXLEN_MASK		= 0xffe00000, // Maximum Length
	TD_TOK_MAXLEN_SHIFT 	= 21,
};

enum {
	TD_PACKET_IN	= 0x69,
	TD_PACKET_OUT	= 0xe1,
	TD_PACKET_SETUP	= 0x2d,
};

typedef volatile struct uhci_td {
	volatile u32 link_ptr;
	volatile union {
		struct {
			u16 actual_len : 10;
			u8 : 5;
			union {
				struct {
					u8 : 1;
					u8 bitstuff_err : 1;
					u8 crc_to : 1;
					u8 nak : 1;
					u8 babble : 1;
					u8 data_buf : 1;
					u8 stalled : 1;
					u8 active : 1;
				};
				u8 status;
			};
			u8 ioc : 1;
			u8 iso : 1;
			u8 ls : 1;
			u8 err_counter : 2;
			u8 spd : 1;
			u8 : 2;
		} _ctrl_sts;
		u32 ctrl_sts;
	};
	volatile union {
		struct {
			u8 pid;
			u8 addr : 7;
			u8 endp : 4;
			u8 data : 1;
			u8 : 1;
			u16 maxlen : 11;
		} _token;
		u32 token; // info
	};
	volatile u32 buffer_ptr;

	u32 free[4];
} __attribute__((aligned(16))) uhci_td_t;

typedef volatile struct uhci_qh {
	volatile u32 head;
	volatile u32 element;
} __attribute__((aligned(16))) uhci_qh_t;

typedef struct uhci {
	pci_hdr* hdr;
	u32* frame_list;
	uhci_td_t* tds;
	uhci_qh_t* qhs;
	u16 io;
} uhci_t;

void uhci_init_controller(pci_hdr* device);
void uhci_send_in(uhci_t* hc, usb_dev* dev, u8* packet, void* output, u8 len);
void uhci_send_address(uhci_t* hc, usb_dev* dev, u8 addr);
