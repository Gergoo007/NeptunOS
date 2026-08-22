#include <arch/amd64/syscalls/syscalls.hh>
#include <arch/arch.hh>
#include <mm/pmm.hh>
#include <gfx/console.hh>
#include <scheduler/scheduler.hh>

extern "C" void syscalls_asm_setup(u64 stack);

void init_syscalls() {
	u64 syscallstack = (u64)pmm_alloc(SYSCALL_STACK_SIZE) + SYSCALL_STACK_SIZE - 128 - 64;
	syscalls_asm_setup(syscallstack);
}

extern "C" void handle_syscall(cpu_state_t* frame) {
	switch (frame->rax) {
		case 0: {
			report("main() returned with %d", (u32)frame->rdi);
			sched_exit_thread();
			break;
		}
		case 1: {
			con_cputs((const char*)frame->rdi);
			break;
		}
	}
}
