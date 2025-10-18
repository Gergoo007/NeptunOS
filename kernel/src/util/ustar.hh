#pragma once

#include <types.hh>
#include <util/storage.hh>

namespace ustar {
	pstruct Header {
		char name[100];
		u64 filemode;
		u64 ownerid;
		u64 groupid;
		u8 filesize[12]; // octal valamilyen oknál fogva
		u8 lastmodif[12]; // UNIX time
		u64 checksum;
		enum : char {
			NORMALFILE = '\0',
			HARDLINK = '1',
			SYMLINK = '2',
			CHARDEV = '3',
			BLOCKDEV = '4',
			DIRECTORY = '5',
			NAMEDPIPE = '6',
		} typeflags;
		char linkedname[100];
		char ustarmagic[6];
		u8 ustarver[2];
		char ownername[32];
		char groupname[32];
		u64 devmajor;
		u64 devminor;
		char prefix[155];

		// Valamiért csak 500 byte a header?
		u8 padding[12];
	};

	struct File {
		const char* name;
		u64 size;
		void* address;
	};

	Opt<File> lookup(void* archive, u64 archivesize, const char* file);
	Vector<File> list(void* archive, u64 archivesize);
}
