#pragma once

#include <types.hh>
#include <util/storage.hh>
#include <fs/fs.hh>

struct device_t;

enum struct PartitionType : u32 {
	OTHER,
	EFI_SYSTEM_PARTITION,
	MICROSOFT_BASIC_DATA,
	MICROSOFT_RESERVED,
	LINUX_FILESYSTEM,
};

struct partition {
	PartitionType type;
	u64 offset;
	u64 size;
	string name;
	device_t& parent;
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

pstruct gpt_entry {
	u8 type[16];
	u8 guid[16];
	u64 start;
	u64 end;
	u64 attr;
	char name[72];
};

struct msd_call_table {
	void (*read)(device_t& msd, u64 lba, u64 sectors, void* buf);
};

void msd_scout(device_t& drive);

void msd_read(device_t& d, u64 offset, u64 bytes, void* buf);
