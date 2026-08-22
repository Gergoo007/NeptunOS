#include <util/stacktrace.hh>
#include <util/ksyms.hh>
#include <util/string.hh>
#include <util/mem.hh>
#include <arch/amd64/paging.hh>
#include <types.hh>

extern "C" void kmain();

void stacktrace(u64 rsp) {
	stackframe_t* s;

	if (rsp == -1ULL)
		asm volatile ("movq %%rbp, %0" : "=r"(s));
	else
		s = (stackframe_t*)rsp;

	printk("Stacktrace:\n");

	u32 max_depth = 8;
	while (s->rip != (u64)kmain && s->rip && max_depth--) {
		extern bool ksyms_been_read;
		if (!ksyms_been_read) {
			printk("[..%08X] 0x%p\n", (u32)s->rip, (void*)s->rip);

			s = s->rbp;
			if (paging_lookup((u64)s) == -1ULL) {
				printk("Invalid frame address %p, exiting...\n", s);
				break;
			}
		} else {
			Elf64_Sym& closest = ksyms[0];

			for (u64 i = 1; i < num_ksyms; i++) {
				if (s->rip >= ksyms[i].st_value && s->rip <= ksyms[i].st_value + ksyms[i].st_size) {
					closest = ksyms[i];
				}
			}

			printk("[..%08X] %s +%llx\n", (u32)s->rip, kstrtab + closest.st_name, s->rip - closest.st_value);

			s = s->rbp;
			if (paging_lookup((u64)s) == -1ULL) {
				printk("Invalid frame address %p, exiting...\n", s);
				break;
			}
		}
	}
}
