#include <util/ustar.hh>
#include <util/string.hh>

namespace ustar {
	Opt<File> lookup(void* archive, u64 archivesize, const char* file) {
		Header* h = (Header*)archive;
		while ((u64)h < (u64)archive + archivesize && !strncmp(h->ustarmagic, "ustar", 5)) {
			u32 size = oct2bin(h->filesize, sizeof(h->filesize));
			if (!strncmp(file, h->name, strlen(file)))
				return Opt<File>(h->name, size, (void*)(h + 1));
			h += align(size, 512) / 512;
		}
		return Opt<File>();
	}

	Vector<File> list(void* archive, u64 archivesize) {
		Vector<File> ret;
		Header* h = (Header*)archive;
		while ((u64)h < (u64)archive + archivesize && !strncmp(h->ustarmagic, "ustar", 5)) {
			u32 size = oct2bin(h->filesize, sizeof(h->filesize));
			ret.emplace(h->name, size, (void*)(h + 1));
			h += align(size, 512) / 512;
		}
		return ret;
	}
}
