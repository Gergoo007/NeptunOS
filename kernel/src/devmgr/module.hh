#pragma once

#include <types.hh>
#include <devmgr/moduleinfo.hh>
#include <util/elf.hh>
#include <util/storage.hh>

namespace modules {
	enum RelocTypes {
		R_X86_64_NONE = 0,       // No reloc
		R_X86_64_64 = 1,         // Direct 64 bit
	};

	struct Module {
		
	};

	extern Vector<Module> modules;
	extern Bitmap bm;

	void register_all();
}
