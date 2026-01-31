#pragma once

#include <types.hh>
#include <devmgr/devmgr.hh>

pstruct fat_bpb {
	u8 jmp[3];
	u8 oem[8];
	u16 bytespersector;
	u8 sectorspercluster;
	u16 num_reserved_sectors;
	u8 num_fats;
	u16 num_root_dir_ents;
	u16 num_sectors;
	u8 media_desc_type;
	u16 sectorsperfat;
	u16 sectorspertrack;
	u16 numheads;
	u32 num_hidden_sectors;
	u32 num_sectors_ext;

	pstruct {
		u32 sectorsperfat;
		u16 flags;
		u16 fatver;
		u32 rootdir_cluster;
		u32 fsinfo_cluster;
		u32 bootbak_cluster;
		u8 zero[12];
		u8 drive_num;
		u8 windows_nt_flags;
		u8 sign;
		u32 volume_id;
		char volume_label[11];
		char identifier[8];
		u8 code[420];
		u16 bootsign; // 0xaa55
	} ext;
};

pstruct fat_fsinfo {
	u32 sign; // 0x41615252
	u8 reserved[480];
	u32 sign2; // 0x61417272
	u32 last_known_free;
	u32 last_known_alloc;
	u8 reserved2[12];
	u32 bootsign; // 0xAA550000
};

pstruct fat_entry {
	u8 simplename[11];
	u8 attrs;
	u8 : 8;
	u8 precisetime;
	pstruct  {
		u16 hour : 5;
		u16 minutes : 6;
		u16 seconds : 5;
	} ctime;
	pstruct {
		u16 year : 7;
		u16 month : 4;
		u16 day : 5;
	} cdate;
	pstruct {
		u16 year : 7;
		u16 month : 4;
		u16 day : 5;
	} adate;
	u16 cluster_hi16;
	pstruct  {
		u16 hour : 5;
		u16 minutes : 6;
		u16 seconds : 5;
	} mtime;
	pstruct {
		u16 year : 7;
		u16 month : 4;
		u16 day : 5;
	} mdate;
	u16 cluster_lo16;
	u32 size;
};

pstruct fat_lfn_entry {
	u8 order : 5;
	u8 : 2;
	u8 last_lfn_ent : 1;
	wchar no1[5];
	u8 attr;
	u8 long_entry_type;
	u8 short_name_checksum;
	wchar no2[6];
	u16 zero;
	wchar no3[2];
};

struct FatAttrs {
	static constexpr u32 RO			= 0x01;
	static constexpr u32 HIDDEN		= 0x02;
	static constexpr u32 SYSTEM		= 0x04;
	static constexpr u32 VOLUME_ID	= 0x08;
	static constexpr u32 DIRECTORY	= 0x10;
	static constexpr u32 ARCHIVE	= 0x20;
	static constexpr u32 LFN		= RO | HIDDEN | SYSTEM | VOLUME_ID;
};
