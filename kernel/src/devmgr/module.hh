#pragma once

#include <types.hh>
#include <util/elf.hh>
#include <util/storage.hh>

enum RelocTypes {
	// R_X86_64_NONE = 0,       // No reloc

	// // S + A
	// R_X86_64_32S = 11,
	// R_X86_64_32 = 10,
	// R_X86_64_64 = 1,         // Direct 64 bit

	// // L + A - P
	// R_X86_64_PLT32 = 4,

	R_X86_64_NONE			= 0,	/* No reloc */
	R_X86_64_64				= 1,	/* Direct 64 bit  */
	R_X86_64_PC32			= 2,	/* PC relative 32 bit signed */
	R_X86_64_GOT32			= 3,	/* 32 bit GOT entry */
	R_X86_64_PLT32			= 4,	/* 32 bit PLT address */
	R_X86_64_COPY			= 5,	/* Copy symbol at runtime */
	R_X86_64_GLOB_DAT		= 6,	/* Create GOT entry */
	R_X86_64_JUMP_SLOT		= 7,	/* Create PLT entry */
	R_X86_64_RELATIVE		= 8,	/* Adjust by program base */
	R_X86_64_GOTPCREL		= 9,	/* 32 bit signed PC relative offset to GOT */
	R_X86_64_32				= 10,	/* Direct 32 bit zero extended */
	R_X86_64_32S			= 11,	/* Direct 32 bit sign extended */
	R_X86_64_16				= 12,	/* Direct 16 bit zero extended */
	R_X86_64_PC16			= 13,	/* 16 bit sign extended pc relative */
	R_X86_64_8				= 14,	/* Direct 8 bit sign extended  */
	R_X86_64_PC8			= 15,	/* 8 bit sign extended pc relative */
	R_X86_64_DTPMOD64		= 16,	/* ID of module containing symbol */
	R_X86_64_DTPOFF64		= 17,	/* Offset in module's TLS block */
	R_X86_64_TPOFF64		= 18,	/* Offset in initial TLS block */
	R_X86_64_TLSGD			= 19,	/* 32 bit signed PC relative offset to two GOT entries for GD symbol */
	R_X86_64_TLSLD			= 20,	/* 32 bit signed PC relative offset to two GOT entries for LD symbol */
	R_X86_64_DTPOFF32		= 21,	/* Offset in TLS block */
	R_X86_64_GOTTPOFF		= 22,	/* 32 bit signed PC relative offset to GOT entry for IE symbol */
	R_X86_64_TPOFF32		= 23,	/* Offset in initial TLS block */
	R_X86_64_PC64			= 24,	/* PC relative 64 bit */
	R_X86_64_GOTOFF64		= 25,	/* 64 bit offset to GOT */
	R_X86_64_GOTPC32		= 26,	/* 32 bit signed pc relative offset to GOT */
	R_X86_64_GOT64			= 27,	/* 64-bit GOT entry offset */
	R_X86_64_GOTPCREL64		= 28,	/* 64-bit PC relative offset to GOT entry */
	R_X86_64_GOTPC64		= 29,	/* 64-bit PC relative offset to GOT */
	R_X86_64_GOTPLT64		= 30, 	/* like GOT64, says PLT entry needed */
	R_X86_64_PLTOFF64		= 31,	/* 64-bit GOT relative offset to PLT entry */
	R_X86_64_SIZE32			= 32,	/* Size of symbol plus 32-bit addend */
	R_X86_64_SIZE64			= 33,	/* Size of symbol plus 64-bit addend */
	R_X86_64_GOTPC32_TLSDESC= 34,	/* GOT offset for TLS descriptor.  */
	R_X86_64_TLSDESC_CALL	= 35,	/* Marker for call through TLS descriptor.  */
	R_X86_64_TLSDESC		= 36,	/* TLS descriptor.  */
	R_X86_64_IRELATIVE		= 37,	/* Adjust indirectly by program base */
	R_X86_64_RELATIVE64		= 38,	/* 64-bit adjust by program base */
	R_X86_64_GOTPCRELX		= 41,	/* Load from 32 bit signed pc relative offset to GOT entry without REX prefix, relaxable.  */
	R_X86_64_REX_GOTPCRELX	= 42,	/* Load from 32 bit signed pc relative offset to GOT entry with REX prefix, relaxable.  */
	R_X86_64_NUM			= 43,
};

// Felső 24 bit: kategória (24 lehetőség)
// Alsó 8 bit: pontos típus (256 lehetőség)
enum struct ModuleTriggerTypes : u32 {
	ANY			= 0x00000000,
	PCIMASK		= 0x00000100,
	USBMASK		= 0x00000200,
	FILESYSTEM	= 0x00000400,
	MISC		= 0x00000800,

	PCI_VENDOR_PRODUCT			= PCIMASK | 1,
	PCI_CLASS_SUBCLASS			= PCIMASK | 2,
	PCI_CLASS_SUBCLASS_PROGIF	= PCIMASK | 3,

	USB_VENDOR_PRODUCT			= USBMASK | 1,
	USB_CLASS_SUBCLASS			= USBMASK | 2,
	USB_CLASS_SUBCLASS_PROGIF	= USBMASK | 3,

	FADT_LEGACY_I8042			= MISC | 1,
	FADT_LEGACY_RTC				= MISC | 2,
};

union ModuleTrigger {
	struct { u8 class_, subclass; }			PCI_CLASS_SUBCLASS;
	struct { u16 vendor, product; }			PCI_VENDOR_PRODUCT;
	struct { u8 class_, subclass, progif; }	PCI_CLASS_SUBCLASS_PROGIF;
	struct {  }								FILESYSTEM;
};

struct module_metadata_t {
	char name[32];
	ModuleTriggerTypes triggertype;
	ModuleTrigger trigger;
};

struct module_t {
	void* content;
	u64 size;
	u64 entry;
	module_metadata_t* metadata;
	bool loaded = false;
};

extern vector<module_t> modules;

struct device_t;
struct partition;
void modules_register_all();
struct filesystem;
bool modules_launch_fs(module_t& m, filesystem& f);
void modules_launch(module_t& m, device_t& dev);
void modules_launch(module_t& m);
