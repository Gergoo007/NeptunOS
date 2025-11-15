#pragma once

#include <types.hh>
#include <util/elf.hh>
#include <util/storage.hh>

enum RelocTypes {
	R_X86_64_NONE = 0,       // No reloc
	R_X86_64_64 = 1,         // Direct 64 bit
};

struct module_t {

};

extern vector<module_t> modules;

void modules_register_all();
