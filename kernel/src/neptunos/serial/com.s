.global outb
.global outw
.global outl

.global inb
.global inw
.global inl

// outb [data] [port]
// out8(data, port)

// Catch up with slower systems
io_wait:
	movb $0x00, %al
	movw $0x80, %dx
	outb %al, (%dx)
	ret

outb:
	movb %dil, %al
	movw %si, %dx
	outb %al, (%dx)
	call io_wait
	ret

outw:
	movw %di, %ax
	movw %si, %dx
	outw %ax, (%dx)
	call io_wait
	ret

outl:
	movl %edi, %eax
	movw %si, %dx
	outl %eax, (%dx)
	call io_wait
	ret

inb:
	movw %di, %dx
	inb %dx, %al
	push %rax
	call io_wait
	pop %rax
	ret

inw:
	movw %di, %dx
	inw (%dx), %ax
	push %rax
	call io_wait
	pop %rax
	ret

inl:
	movw %di, %dx
	inl (%dx), %eax
	push %rax
	call io_wait
	pop %rax
	ret
