.global teszt
.extern khang

.section .bss
kstack:
	.quad 0

.section .text
on_syscall:
	cmp $0, %eax
	je .exit
.printk:
	push %r11
	push %rcx
	call printk
	pop %rcx
	pop %r11
	sysretq

.exit:
	movq kstack(%rip), %rsp

	# Kernel regiszterek syscall előtt
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rsi
	pop %rdi
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax
	pop %rbp

	jmp khang

# %rdi: entry
# %rsi: uj stack
teszt:
	# STAR MSR
	movq $0xC0000081, %rcx
	# STAR 63:48 + 16: user CS
	# STAR 63:48 + 08: user SS
	mov $0x00000000, %rax
	// mov $0x000b0008, %edx
	mov $0x00100008, %edx
	wrmsr

	# LSTAR MSR
	movq $0xC0000082, %rcx
	# LSTAR: syscall %rip
	movabs $on_syscall, %rdx
	shr $32, %rdx
	movabs $on_syscall, %rax
	wrmsr

	# %rip
	mov %rdi, %rcx
	# %eflags
	mov $0x202, %r11
	// mov $0x002, %r11

	# Többi szegmens beállítása 0x18-ra (offsetof gdt->udata)
	mov $0x1b, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	swapgs

	movq %rsp, kstack(%rip)

	push %rbp
	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rdi
	push %rsi
	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15

	# Felhasználói stack
	mov %rsi, %rsp

	sysretq
