#pragma once

#include <types.hh>
#include <util/storage.hh>
#include <fs/fs.hh>

struct Device;

enum struct PartitionType : u32 {
	OTHER,
	EFI_SYSTEM_PARTITION,
	MICROSOFT_BASIC_DATA,
	MICROSOFT_RESERVED,
	LINUX_FILESYSTEM,
};

struct Partition {
	PartitionType type;
	u64 offset;
	u64 size;
	String name;
	Device& parent;
};

pstruct gpt {
	u64 magic;
	u32 rev;
	u32 size;
	u32 crc32;
	u32 : 32;
	u64 lba;
	u64 altlba;
	u64 firstlba;
	u64 lastlba;
	u8 guid[16];
	u64 startlba;
	u32 num_parts;
	u32 entry_size;
	u32 partscrc32;
};

pstruct GPTEntry {
	u8 type[16];
	u8 guid[16];
	u64 start;
	u64 end;
	u64 attr;
	char name[72];
};

struct msd_call_table {
	void (*read)(Device& msd, u64 lba, u64 sectors, void* buf);
};

void msd_scout(Device& drive);

void msd_read(Device& d, u64 offset, u64 bytes, void* buf);
