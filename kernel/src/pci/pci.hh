#pragma once

#include <types.hh>

namespace pci {
	pstruct MCFG {
		pstruct SDT {
			u32 sign;
			u32 length;
			u8 rev;
			u8 checksum;
			char oemid[6];
			char oemtableid[8];
			u32 oem_rev;
			u32 creatorid;
			u32 creatorrev;
		} sdt;
		u64 : 64;

		pstruct {
			void* base;
			u16 seggroup;
			u8 busstart;
			u8 busend;
			u32 : 32;
		} cfg_spaces[0];
	};

	typedef punion pci_hdr {
		pstruct {
			u16 vendor;
			u16 product;
			u16 cmd;
			u16 status;
			u8 rev_id;
			u8 prog_if;
			union {
				struct {
					u8 subclass;
					u8 _class;
				};
				u16 combclass;
			};
			u8 cache_size;
			u8 latency_timer;
			u8 hdr_type;
			u8 bist;

			union {
				struct {
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

				struct {
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

				struct {
					// TODO
				} type2;
			};
		};
		pstruct {
			u32 dword[16];
		};
	} pci_hdr;

	struct Register {
		u32 offset;
		u32 bits;
	};

	struct Regs {
		static constexpr Register VENDOR 		= { 0x00,	16	};
		static constexpr Register PRODUCT 		= { 0x02,	16	};
		static constexpr Register CMD 			= { 0x04,	16	};
		static constexpr Register STS 			= { 0x06,	16	};
		static constexpr Register REVID 		= { 0x08,	8	};
		static constexpr Register PROGIF 		= { 0x09,	8	};
		static constexpr Register SUBCLASS 		= { 0x0a,	8	};
		static constexpr Register CLASS 		= { 0x0b,	8	};
		static constexpr Register HDRTYPE 		= { 0x0e,	8	};
		static constexpr Register BAR0 			= { 0x10,	32	};
		static constexpr Register BAR1 			= { 0x14,	32	};
		static constexpr Register BAR2 			= { 0x18,	32	};
		static constexpr Register PRIMARYBUS 	= { 0x18,	8	};
		static constexpr Register SECONDARYBUS 	= { 0x19,	8	};
		static constexpr Register BAR3 			= { 0x1c,	32	};
		static constexpr Register BAR4 			= { 0x20,	32	};
		static constexpr Register BAR5 			= { 0x24,	32	};
		static constexpr Register BAR6 			= { 0x28,	32	};
		static constexpr Register INTLINE 		= { 0x3c,	8	};
		static constexpr Register INTPIN 		= { 0x3d,	8	};
		static constexpr Register MINGRANT 		= { 0x3e,	8	};
		static constexpr Register MAXLATEN 		= { 0x3f,	8	};
	};

	extern MCFG* mcfg;

	void init();
}
