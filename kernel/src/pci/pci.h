#pragma once

#include <graphics/console.h>

#pragma pack(1)

typedef struct pci_hdr {
	u16 vendor;
	u16 product;
	u16 cmd;
	u16 status;
	u8 rev_id;
	u8 prog_if;
	u8 subclass;
	u8 class;
	u8 cache_size;
	u8 latency_timer;
	u8 hdr_type;
	u8 bist;

	union {
		struct {
			u32 bar0;
			u32 bar1;
			u32 bar2;
			u32 bar3;
			u32 bar4;
			u32 bar5;
			u32 cardbus_cis;
			u16 subsys_vendor;
			u16 subsys_id;
			u32 expansion_rom_addr;
			u8 capabilities;
			u32 : 24;
			u32 : 32;
			u8 int_line;
			u8 int_pin;
			u8 min_grant;
			u8 max_latency;
		} type0;

		struct {
			u32 bar0;
			u32 bar1;
			u8 primary_bus_num;
			u8 secondary_bus_num;
			u8 subordinate_bus_num;
			u8 secondary_latency_timer;
			u8 io_base;
			u8 io_limit;
			u16 secondary_status;
			u16 mem_base;
			u16 mem_limit;
			u16 prefetch_mem_base;
			u16 prefetch_mem_limit;
			u32 prefetchable_base_upper;
			u32 prefetchable_limit_upper;
			u16 io_base_upper;
			u16 io_limit_upper;
			u8 capabilities;
			u32 : 24;
			u32 expansion_rom_addr;
			u8 int_line;
			u8 int_pin;
			u16 bridge_ctrl;
		} type1;

		struct {
			// TODO
		} type2;
	};
} pci_hdr_t;

#include <usb/uhci.h>

void pci_add_device(pci_hdr_t* dev);
