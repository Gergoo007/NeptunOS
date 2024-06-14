#include <arch/x86/interrupts.h>

#include <graphics/console.h>

#include "lib/symlookup.h"

#define print_reg(st, reg) printk("%s: %p ", #reg, st->reg)
#define sprint_reg(st, reg) sprintk("%s: %p ", #reg, st->reg)

_attr_saved_regs void exception_handler(regs* frame) {
	asm volatile ("cli");

	switch (frame->exc) {
		default: {
			sprintk("!! EXC 0x%x [err: %x]\n\r", frame->exc, frame->err);
			sprint_reg(frame, rax); sprint_reg(frame, rbx);
			sprintk("\n\r");
			sprint_reg(frame, rcx); sprint_reg(frame, rdx);
			sprintk("\n\r");
			sprint_reg(frame, rdi); sprint_reg(frame, rsi);
			sprintk("\n\r");
			sprint_reg(frame, rdx); sprint_reg(frame, cr2);
			sprintk("\n\r");
			sprint_reg(frame, rip); sprint_reg(frame, rfl);
			sprintk("\n\r");
			sprint_reg(frame, rsp); sprint_reg(frame, rbp);
			sprintk("\n\r");

			// Stack trace
			char buf[256];
			stacktrace((stackframe*)frame->rbp, buf);
			sprintk("%s", buf);

			break;
		}
	}

	switch (frame->exc) {
		default: {
			printk("!! EXC 0x%x [err: %x]\n", frame->exc, frame->err);
			print_reg(frame, rax); print_reg(frame, rbx);
			printk("\n");
			print_reg(frame, rcx); print_reg(frame, rdx);
			printk("\n");
			print_reg(frame, rdi); print_reg(frame, rsi);
			printk("\n");
			print_reg(frame, rdx); print_reg(frame, cr2);
			printk("\n");
			print_reg(frame, rip); print_reg(frame, rfl);
			printk("\n");
			print_reg(frame, rsp); print_reg(frame, rbp);
			printk("\n");

			// Stack trace
			char buf[256];
			stacktrace((stackframe*)frame->rbp, buf);
			printk("%s", buf);

			break;
		}
	}

	asm volatile ("hlt");
}
