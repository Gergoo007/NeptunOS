.global syscalls_asm_setup

.extern handle_syscall

.text
on_syscall:
	movq %rsp, ursp(%rip)
	movq %rbp, urbp(%rip)

	movq krsp(%rip), %rsp
	movq krbp(%rip), %rbp

	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rdi
	push %rsi
	push %rbp

	movq %cr2, %rax
	push %rax

	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15
	mov %cr3, %rax
	push %rax

	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss

	movq %rsp, %rdi
	movabsq $handle_syscall, %rax
	cld
	call *%rax

	pop %r15 # cr3
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rax # cr2
	pop %rbp
	pop %rsi
	pop %rdi
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax

	movq ursp(%rip), %rsp
	movq urbp(%rip), %rbp

	sysretq

.bss
ursp: .quad 0
urbp: .quad 0
krsp: .quad 0
krbp: .quad 0

.text
syscalls_asm_setup:
	movq %rdi, krsp(%rip)
	movq %rdi, krbp(%rip)

	# syscall utasítás engedélyezése
	movq $0xC0000080, %rcx
	rdmsr
	or $1, %eax
	wrmsr

	# STAR MSR
	movq $0xC0000081, %rcx
	# STAR 63:48 + 16: user CS
	# STAR 63:48 + 08: user SS
	mov $0x00000000, %eax
	mov $0x00130008, %edx
	wrmsr

	# LSTAR MSR
	movq $0xC0000082, %rcx
	# LSTAR: syscall %rip
	movabs $on_syscall, %rdx
	shr $32, %rdx
	movabs $on_syscall, %rax
	wrmsr

	ret
