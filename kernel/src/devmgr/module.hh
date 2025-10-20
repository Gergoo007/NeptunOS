#pragma once

#include <types.hh>
#include <devmgr/moduleinfo.hh>
#include <util/elf.hh>
#include <util/storage.hh>

namespace modules {
	struct Module {
		
	};

	extern Vector<Module> modules;

	void register_all();
}
