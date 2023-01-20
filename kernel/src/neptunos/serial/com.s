.global outb
.global outw
.global outl

.global inb
.global inw
.global inl

// outb [data] [port]
// out8(data, port)

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
	inb (%dx), %al
	ret

inw:
	movw %di, %dx
	inw (%dx), %ax
	ret

inl:
	movw %di, %dx
	inl (%dx), %eax
	ret
