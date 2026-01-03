.global arch_cpu_state_load

.extern sse_state

# %rdi: cpu_state_t {
# 	u64 r15;
# 	u64 r14;
# 	u64 r13;
# 	u64 r12;
# 	u64 r11;
# 	u64 r10;
# 	u64 r9;
# 	u64 r8;
# 	u64 cr2;
# 	u64 rbp;
# 	u64 rsi;
# 	u64 rdi;
# 	u64 rdx;
# 	u64 rcx;
# 	u64 rbx;
# 	u64 rax;
# 	u64 exc;
# 	u64 rflexc;
# 	u64 err;
# 	u64 rip;
# 	u64 cs;
# 	u64 rfl;
# 	u64 rsp;
# 	u64 ss;
# }
.text
arch_cpu_state_load:
	mov %rdi, %rsp

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

	fxrstor64 sse_state

	# hibakód + vektor
	add $0x10, %rsp

	popfq
	iretq

.data
str0:
	.asciz "asd turi %p %p %p %p\n"
