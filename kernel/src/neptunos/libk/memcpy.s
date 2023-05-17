.global memcpy

// if the data to be moved is bigger than or equal to 16 bytes,
// use xmm0, otherwise rax or eax or ax or al
// src: %rsi
// dest: %rdi
// size (in bytes): %rdx

memcpy:
.check_xmm:
	cmp $0x10, %rdx
	jge .xmm
.check_rax:
	cmp $0x8, %rdx
	jge .rax
	cmp $0, %rdx
	jne .al
	movq %rdi, %rax
	ret
.al:
	sub $0x1, %rdx
	movb (%rsi, %rdx, 1), %al
	movb %al, (%rdi, %rdx, 1)
	cmp $0, %rdx
	jne .al
	ret
.rax:
	sub $0x8, %rdx
	movq (%rsi, %rdx, 1), %rax
	movq %rax, (%rdi, %rdx, 1)
	jmp .check_rax
.xmm:
	sub $0x10, %rdx
	movups (%rsi, %rdx, 1), %xmm0
	movups %xmm0, (%rdi, %rdx, 1)
	jmp .check_xmm
