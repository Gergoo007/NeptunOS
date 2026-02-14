#pragma once

#include <types.hh>

pstruct mcfg_t {
	pstruct {
		u32 sign;
		u32 length;
		u8 rev;
		u8 checksum;
		char oemid[6];
		char oemtableid[8];
		u32 oem_rev;
		u32 creatorid;
		u32 creatorrev;
	};
	u64 : 64;

	pstruct {
		void* base;
		u16 seggroup;
		u8 busstart;
		u8 busend;
		u32 : 32;
	} cfg_spaces[0];
};

punion pci_hdr_t {
	pstruct {
		u16 vendor;
		u16 product;
		u16 cmd;
		u16 status;
		u8 rev_id;
		u8 prog_if;
		punion {
			pstruct {
				u8 subclass;
				u8 _class;
			};
			u16 combclass;
		};
		u8 cache_size;
		u8 latency_timer;
		u8 hdr_type;
		u8 bist;

		punion {
			pstruct {
				u32 bars[6];
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

			pstruct {
				u32 bars[2];
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

			pstruct {
				// TODO
			} type2;
		};
	};
	pstruct {
		u32 dword[16];
	};
};

punion pci_msi_addr {
	pstruct {
		u32 xx : 2;
		u32 dest_mode : 1;
		u32 redir_hint : 1;
		u32 z0 : 8;
		u32 destination : 8;
		u32 mmio : 12;
	};
	u32 raw;
};

punion pci_msi_data {
	pstruct {
		u32 vector : 8;
		u32 delivery : 3;
		u32 z0 : 3;
		u32 active_hi : 1;
		u32 lvl_trig : 1;
		u32 z1 : 16;
		u32 z2 : 32;
	};
	u32 raw;
};

punion pci_msi_control {
	pstruct {
		u32 enable : 1;
		u32 multi_msg_capable : 3;
		u32 multi_msg_enable : 3;
		u32 bits64 : 1;
		u32 per_vector_masking : 1;
		u32 : 7;
	};
	u32 raw;
};

struct pci_register_t {
	u32 offset;
	u32 bits;

	constexpr pci_register_t(u8 barnum): offset(0x10 + barnum * 4), bits(32) {  };
	constexpr pci_register_t(u32 o, u32 b): offset(o), bits(b) {  };
};

struct PciRegs {
	static constexpr pci_register_t VENDOR 		= { 0x00,	16	};
	static constexpr pci_register_t PRODUCT 	= { 0x02,	16	};
	static constexpr pci_register_t CMD 		= { 0x04,	16	};
	static constexpr pci_register_t STS 		= { 0x06,	16	};
	static constexpr pci_register_t REVID 		= { 0x08,	8	};
	static constexpr pci_register_t PROGIF 		= { 0x09,	8	};
	static constexpr pci_register_t SUBCLASS 	= { 0x0a,	8	};
	static constexpr pci_register_t CLASS 		= { 0x0b,	8	};
	static constexpr pci_register_t HDRTYPE 	= { 0x0e,	8	};
	static constexpr pci_register_t BAR0 		= { 0x10,	32	};
	static constexpr pci_register_t BAR1 		= { 0x14,	32	};
	static constexpr pci_register_t BAR2 		= { 0x18,	32	};
	static constexpr pci_register_t PRIMARYBUS 	= { 0x18,	8	};
	static constexpr pci_register_t SECONDARYBUS= { 0x19,	8	};
	static constexpr pci_register_t BAR3 		= { 0x1c,	32	};
	static constexpr pci_register_t BAR4 		= { 0x20,	32	};
	static constexpr pci_register_t BAR5 		= { 0x24,	32	};
	static constexpr pci_register_t BAR6 		= { 0x28,	32	};
	static constexpr pci_register_t CAPSPTR		= { 0x34,	8	};
	static constexpr pci_register_t INTLINE 	= { 0x3c,	8	};
	static constexpr pci_register_t INTPIN 		= { 0x3d,	8	};
	static constexpr pci_register_t MINGRANT 	= { 0x3e,	8	};
	static constexpr pci_register_t MAXLATEN 	= { 0x3f,	8	};
};

struct pci_bar {
	u64 addr;
	bool io;
};

extern mcfg_t* mcfg;

void pci_init();
struct device_t;
u32 pci_read(device_t& dev, pci_register_t reg);
void pci_write(device_t& dev, pci_register_t reg, u32 data);
void pci_setup_cmd_reg(device_t& dev);
pci_bar pci_prepare_bar(device_t& dev, u8 barnum);
void pci_enable_msi(device_t& d, u8 vector);
