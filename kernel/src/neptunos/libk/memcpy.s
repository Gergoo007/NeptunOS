.global memcpy_comp
.global memcpy_sse
.global memcpy_avx2

// if the data to be moved is bigger than or equal to 16 bytes,
// use xmm0, otherwise rax or eax or ax or al
// src: %rsi
// dest: %rdi
// size (in bytes): %rdx

// compatibility variant
memcpy_comp:
memcpy_comp.check_xmm:
	cmp $0x10, %rdx
	jge memcpy_comp.xmm
memcpy_comp.check_rax:
	cmp $0x8, %rdx
	jge memcpy_comp.rax
	cmp $0, %rdx
	jne memcpy_comp.al
	movq %rdi, %rax
	ret
memcpy_comp.al:
	sub $0x1, %rdx
	movb (%rsi, %rdx, 1), %al
	movb %al, (%rdi, %rdx, 1)
	cmp $0, %rdx
	jne memcpy_comp.al
	ret
memcpy_comp.rax:
	sub $0x8, %rdx
	movq (%rsi, %rdx, 1), %rax
	movq %rax, (%rdi, %rdx, 1)
	jmp memcpy_comp.check_rax
memcpy_comp.xmm:
	sub $0x10, %rdx
	movups (%rsi, %rdx, 1), %xmm0
	movups %xmm0, (%rdi, %rdx, 1)
	jmp memcpy_comp.check_xmm

memcpy_sse:
memcpy_sse.check_xmm:
	cmp $0x10, %rdx
	jge memcpy_sse.xmm
memcpy_sse.check_rax:
	cmp $0x8, %rdx
	jge memcpy_sse.rax
	cmp $0, %rdx
	jne memcpy_sse.al
	movq %rdi, %rax
	ret
memcpy_sse.al:
	sub $0x1, %rdx
	movb (%rsi, %rdx, 1), %al
	movb %al, (%rdi, %rdx, 1)
	cmp $0, %rdx
	jne memcpy_sse.al
	ret
memcpy_sse.rax:
	sub $0x8, %rdx
	movq (%rsi, %rdx, 1), %rax
	movq %rax, (%rdi, %rdx, 1)
	jmp memcpy_sse.check_rax
memcpy_sse.xmm:
	sub $0x10, %rdx
	movups (%rsi, %rdx, 1), %xmm0
	movups %xmm0, (%rdi, %rdx, 1)
	jmp memcpy_sse.check_xmm

memcpy_avx2:
memcpy_avx2.check_ymm:
	cmp $0x20, %rdx
	jge memcpy_avx2.ymm
memcpy_avx2.check_xmm:
	cmp $0x10, %rdx
	jge memcpy_avx2.xmm
memcpy_avx2.check_rax:
	cmp $0x8, %rdx
	jge memcpy_avx2.rax
	cmp $0, %rdx
	jne memcpy_avx2.al
	movq %rdi, %rax
	ret
memcpy_avx2.al:
	sub $0x1, %rdx
	movb (%rsi, %rdx, 1), %al
	movb %al, (%rdi, %rdx, 1)
	cmp $0, %rdx
	jne memcpy_avx2.al
	ret
memcpy_avx2.rax:
	sub $0x8, %rdx
	movq (%rsi, %rdx, 1), %rax
	movq %rax, (%rdi, %rdx, 1)
	jmp memcpy_avx2.check_rax
memcpy_avx2.xmm:
	sub $0x10, %rdx
	movups (%rsi, %rdx, 1), %xmm0
	movups %xmm0, (%rdi, %rdx, 1)
	jmp memcpy_avx2.check_xmm
memcpy_avx2.ymm:
	sub $0x20, %rdx
	vmovups (%rsi, %rdx, 1), %ymm0
	vmovups %ymm0, (%rdi, %rdx, 1)
	jmp memcpy_avx2.check_xmm
