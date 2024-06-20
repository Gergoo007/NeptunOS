.global loader_run

loader_run:
	mov %rdi, %rax
	call *%rax
	ret
