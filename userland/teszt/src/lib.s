.global puts
.global _start

.extern main

_start:
	call main

	mov %eax, %edi
	mov $0, %eax
	syscall
