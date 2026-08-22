.global sse_init

sse_init:
	mov %cr0, %rax
	and $~(1 << 2), %rax      # clear EM (bit 2)
	or  $(1 << 1), %rax       # set MP (bit 1)
	mov %rax, %cr0

	mov %cr4, %rax
	or  $(1 << 9), %rax       # set OSFXSR (bit 9)
	or  $(1 << 10), %rax      # set OSXMMEXCPT (bit 10)
	or  $(1 << 18), %rax      # set OSXSAVE (bit 18)
	// or 	$(1 << 16), %rax
	mov %rax, %cr4

	// # van AVX meg XSAVE support?
	// mov $1, %eax
	// cpuid
	// # ECX:28 && ECX:26
	// test $(1 << 28), %ecx
	// jnz .sse_init.end
	// test $(1 << 26), %ecx
	// jnz .sse_init.end
	// mov $0xabcdef, %eax
	// jmp .

	# AVX
    push %rcx
	push %rdx

	xor %ecx, %ecx
    xgetbv # Load XCR0 register
    orl $0b111, %eax # Set AVX, SSE, X87 bits
    xsetbv # Save back to XCR0

    pop %rdx
    pop %rcx

.sse_init.end:
    ret
