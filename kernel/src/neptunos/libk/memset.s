.global memset

// if the data to be moved is bigger than or equal to 16 bytes,
// use xmm0, otherwise rax or eax or ax or al
// src: %rsi
// dest: %rdi
// size (in bytes): %rdx

memset:
.check_xmm:
	cmp $0x10, %rdx
	movl %esi, %eax
	movd %eax, %xmm0
	jge .xmm
.check_rax:
	cmp $0x8, %rdx
	movq $0, %rax
	movl %esi, %eax
	jge .rax

	cmp $0, %rdx
	movb %sil, %al
	jne .al
	movq %rdi, %rax
	ret
.al:
	sub $0x1, %rdx
	movb %al, (%rdi, %rdx, 1)
	cmp $0, %rdx
	jne .al
	ret
.rax:
	sub $0x8, %rdx
	movq %rax, (%rdi, %rdx, 1)
	jmp .check_rax
.xmm:
	sub $0x10, %rdx
	movups %xmm0, (%rdi, %rdx, 1)
	jmp .check_xmm
