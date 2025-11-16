#pragma once

#include <types.hh>
#include <util/elf.hh>
#include <util/storage.hh>

enum RelocTypes {
	R_X86_64_NONE = 0,       // No reloc
	R_X86_64_64 = 1,         // Direct 64 bit
};

enum struct ModuleTriggerTypes : i32 {
	ANY = -1,
	PCI_CLASS_SUBCLASS = 0,
	PCI_VENDOR_PRODUCT = 1,
	PCI_CLASS_SUBCLASS_PROGIF = 2,
};

union ModuleTrigger {
	struct { u8 class_, subclass; }			PCI_CLASS_SUBCLASS;
	struct { u16 vendor, product; }			PCI_VENDOR_PRODUCT;
	struct { u8 class_, subclass, progif; }	PCI_CLASS_SUBCLASS_PROGIF;
};

struct module_metadata_t {
	char name[32];
	ModuleTriggerTypes triggertype;
	ModuleTrigger trigger;
};

struct module_t {
	void* content;
	u64 size;
	module_metadata_t* metadata;
};

extern vector<module_t> modules;

void modules_register_all();
void modules_load(const module_t& m);
