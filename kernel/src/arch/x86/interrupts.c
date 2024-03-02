#include <arch/x86/interrupts.h>

#include <graphics/console.h>

#include "lib/symlookup.h"

#define print_reg(st, reg) printk("%s: %p ", #reg, st->reg)

_attr_saved_regs void exception_handler(regs_t* frame) {
	switch (frame->exc) {
		default: {
			printk("!! EXC 0x%x [err: %x]\n", frame->exc, frame->err);
			print_reg(frame, rax); print_reg(frame, rbx);
			kputc('\n');
			print_reg(frame, rcx); print_reg(frame, rdx);
			kputc('\n');
			print_reg(frame, rdi); print_reg(frame, rsi);
			kputc('\n');
			print_reg(frame, rdx); print_reg(frame, cr2);
			kputc('\n');
			print_reg(frame, rip); print_reg(frame, rflags);
			kputc('\n');
			print_reg(frame, rsp); print_reg(frame, rbp);
			kputc('\n');

			// Stack trace
			char buf[256];
			stacktrace((stackframe_t*)frame->rbp, buf);
			kputs(buf);

			break;
		}
	}

	asm volatile ("cli");
	asm volatile ("hlt");
}
