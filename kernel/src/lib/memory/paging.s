[bits 64]

global setup_pae
global enable_paging

setup_pae:
	push rax
	mov rax, cr4
	bts rax, 5
	btc rax, 12
	mov cr4, rax
	pop rax
	ret

enable_paging:
	mov cr3, rdi
	mov dx, 0x3f8
	mov al, 'c'
	out dx, al
	ret
