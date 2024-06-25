.global teszt

user_func:
	jmp .

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
	// movq $0xC0000082, %rcx
	// # LSTAR: %rip
	// mov $0x89abcdef, %eax
	// mov $0x01234567, %edx
	// wrmsr

	// # ???
	// mov $0xc0000084, %rcx
	// xor %rax, %rax
	// wrmsr

	# %rip
	// mov %rdi, %rcx
	movabs $user_func, %rcx
	# %eflags
	mov $0x202, %r11
	// mov $0x002, %r11

	// mov %rsi, %rsp
	
	# Többi szegmens beállítása 0x18-ra (offsetof gdt->udata)
	mov $0x1b, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	swapgs

	sysretq
