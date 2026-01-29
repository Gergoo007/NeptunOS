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
};

pstruct fat_extbootrec {
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
