#include <lib/symlookup.h>

u64 abs64(i64 num) {
	num &= ~(1ULL << 63);
	return num;
}

char* sym_at_addr(u64 addr) {
	elf64_sym* sym = (elf64_sym*)info->symtab;
	while (sym->st_value != addr) { sym++; }
	return (char*)info->strtab1+sym->st_name;
}

u64 sym_nearest_sym(u64 addr) {
	elf64_sym* sym = (elf64_sym*)info->symtab;

	u64 diff = 0xffffffffffffffff;

	for (u8 i = 0; i < info->symtab_entries; i++, sym++) {
		if (sym->st_value == addr) return addr;
		if (sym->st_value > addr) continue;

		if (addr - sym->st_value < diff) {
			diff = addr - sym->st_value;
		}
	}

	return (addr - diff);
}
