#pragma once

#include <types.hh>

namespace acpi {
	struct SDT {
		char sign[4];
		u32 length;
		u8 rev;
		u8 checksum;
		char oemid[6];
		char oemtableid[8];
		u32 oem_rev;
		u32 creatorid;
		u32 creatorrev;
	};

	punion RootTable {
		pstruct {

		} rsdt;
		pstruct {

		} xsdt;
	};

	bool validate(SDT* table);
	void init();
}
