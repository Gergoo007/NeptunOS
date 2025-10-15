.global sse_init

sse_init:
	mov %cr0, %rax
	and $~(1 << 2), %rax      # clear EM (bit 2)
	or  $(1 << 1), %rax       # set MP (bit 1)
	mov %rax, %cr0

	mov %cr4, %rax
	or  $(1 << 9), %rax       # set OSFXSR (bit 9)
	or  $(1 << 10), %rax      # set OSXMMEXCPT (bit 10)
	mov %rax, %cr4
	ret
