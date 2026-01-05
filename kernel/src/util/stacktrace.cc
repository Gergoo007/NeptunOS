#include <util/stacktrace.hh>
#include <util/ksyms.hh>
#include <util/string.hh>
#include <util/mem.hh>
#include <arch/amd64/paging.hh>
#include <types.hh>

extern "C" void kmain();

void stacktrace(u64 rsp, void (*_printk)(const char* fmt, ...)) {
	stackframe_t* s;

	if (rsp == -1ULL)
		asm volatile ("movq %%rbp, %0" : "=r"(s));
	else
		s = (stackframe_t*)rsp;

	_printk("Stacktrace:\n");

	while (s->rip != (u64)kmain && s->rip) {
		extern bool ksyms_been_read;
		if (!ksyms_been_read) {
			_printk("[..%08X] 0x%p\n", (u32)s->rip, s->rip);

			s = s->rbp;
			if (paging_lookup((u64)s) == -1ULL) {
				_printk("Invalid frame address %p, exiting...\n", s);
				break;
			}
		} else {
			Elf64_Sym& closest = ksyms[0];

			for (u64 i = 1; i < num_ksyms; i++) {
				if (s->rip >= ksyms[i].st_value && s->rip <= ksyms[i].st_value + ksyms[i].st_size) {
					closest = ksyms[i];
				}
			}

			_printk("[..%08X] %s +%llx\n", (u32)s->rip, kstrtab + closest.st_name, s->rip - closest.st_value);

			s = s->rbp;
			if (paging_lookup((u64)s) == -1ULL) {
				_printk("Invalid frame address %p, exiting...\n", s);
				break;
			}
		}
	}
}
