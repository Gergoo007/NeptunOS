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
		static constexpr Register HDRTYPE 		= { 0x0d,	8	};
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

	// enum struct Regs : Register {
	// 	VENDOR_16 = 0,
	// 	PRODUCT_16 = 2,
	// 	CMD_16 = 4,
	// 	STS_16 = 6,
	// 	REVID_8 = 8,
	// 	PROGIF_8 = 9,
	// 	SUBCLASS_8 = 10,
	// 	CLASS_8 = 11,
	// 	HDRTYPE = 14,
	// 	BAR0 = 16,
	// 	BAR1 = 20,
	// 	BAR2 = 24,
	// 	BAR3 = 28,
	// 	BAR4 = 32,
	// 	BAR5 = 36,
	// 	BAR6 = 40,
	// 	INTLINE_INTPIN_MINGRANT_MAXLATEN = 60,
	// };

	extern MCFG* mcfg;

	void init();
}
