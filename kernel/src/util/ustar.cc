#include <util/ustar.hh>
#include <util/string.hh>

Opt<ustar_file> ustar_lookup(void* archive, u64 archivesize, const char* file) {
	ustar_header* h = (ustar_header*)archive;
	while ((u64)h < (u64)archive + archivesize && !strncmp(h->ustarmagic, "ustar", 5)) {
		u32 size = oct2bin(h->filesize, sizeof(h->filesize));
		if (!strncmp(file, h->name, strlen(file)))
			return Opt<ustar_file>(h->name, size, (void*)(h + 1));
		h += align(size, 512) / 512;
	}
	return Opt<ustar_file>();
}

Vector<ustar_file> ustar_list(void* archive, u64 archivesize) {
	Vector<ustar_file> ret;
	ustar_header* h = (ustar_header*)archive;
	while ((u64)h < (u64)archive + archivesize && !strncmp(h->ustarmagic, "ustar", 5)) {
		u32 size = oct2bin(h->filesize, sizeof(h->filesize) - 1);
		ret.emplace_back(h->name, size, (void*)(h + 1));
		h += align(size, 512) / 512 + 1;
	}
	return ret;
}
