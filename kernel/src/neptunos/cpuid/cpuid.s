.global cpuid_get_vendor
.global cpuid_get_version
.global cpuid_sse
.global cpuid_sse2
.global cpuid_sse3
.global cpuid_sse41
.global cpuid_sse42

.global cpuid_avx
.global cpuid_avx2
.global cpuid_avx512f

.global cpuid_apic

// rdi: pointer to string into which vendor gets stored
// returns: highest eax value when using cpuid
cpuid_get_vendor:
	movl $0x0, %eax
	movl $0x0, %ecx
	cpuid
	movl %ebx, (%rdi)
	movl %edx, 4(%rdi)
	movl %ecx, 8(%rdi)
	movb $0x0, 13(%rdi)
	ret

cpuid_get_version:
	movl $0x1, %eax
	movl $0x0, %ecx
	cpuid
	ret

cpuid_sse:
	movl $0x1, %eax
	movl $0x0, %ecx
	cpuid
	movl %edx, %eax
	shr $25, %eax
	and $1, %eax
	ret

cpuid_sse2:
	movl $0x1, %eax
	movl $0x0, %ecx
	cpuid
	movl %edx, %eax
	shr $26, %eax
	and $1, %eax
	ret

cpuid_sse3:
	movl $0x1, %eax
	movl $0x0, %ecx
	cpuid
	movl %ecx, %eax
	and $1, %eax
	ret

cpuid_sse41:
	movl $0x1, %eax
	movl $0x0, %ecx
	cpuid
	movl %ecx, %eax
	shr $19, %eax
	and $1, %eax
	ret

cpuid_sse42:
	movl $0x1, %eax
	movl $0x0, %ecx
	cpuid
	movl %ecx, %eax
	shr $20, %eax
	and $1, %eax
	ret

cpuid_avx:
	movl $0x1, %eax
	movl $0x0, %ecx
	cpuid
	movl %ecx, %eax
	shr $28, %eax
	and $1, %eax
	ret

cpuid_avx2:
	movl $0x7, %eax
	movl $0x0, %ecx
	cpuid
	movl %ebx, %eax
	shr $5, %eax
	and $1, %eax
	ret

cpuid_avx512f:
	movl $0x7, %eax
	movl $0x0, %ecx
	cpuid
	movl %ebx, %eax
	shr $16, %eax
	and $1, %eax
	ret

cpuid_apic:
	movl $0x1, %eax
	movl $0x0, %ecx
	cpuid
	movl %edx, %eax
	shr $9, %eax
	and $1, %eax
	ret
