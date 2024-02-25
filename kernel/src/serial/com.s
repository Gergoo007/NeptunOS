.global outb
.global outw
.global outl

.global inb
.global inw
.global inl

outb:
	movb %dil, %al
	movw %si, %dx
	outb %al, (%dx)
	ret

outw:
	movw %di, %ax
	movw %si, %dx
	outw %ax, (%dx)
	ret

outl:
	movl %edi, %eax
	movw %si, %dx
	outl %eax, (%dx)
	ret

inb:
	movw %di, %dx
	inb %dx, %al
	push %rax
	pop %rax
	ret

inw:
	movw %di, %dx
	inw (%dx), %ax
	push %rax
	pop %rax
	ret

inl:
	movw %di, %dx
	inl (%dx), %eax
	push %rax
	pop %rax
	ret
