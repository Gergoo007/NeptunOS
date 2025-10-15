#include <util/stacktrace.hh>
#include <types.hh>

extern "C" void kmain();

void stacktrace() {
	Stackframe* s;
	asm volatile ("movq %%rsp, %0" : "=r"(s));
	while (s->rip != (u64)kmain) {
		printk("Stackframe address: %p\n", (void*)s->rip);
		printk("Stackframe address: %p\n", (void*)s->rbp);
		s = s->rbp;
	}
}
