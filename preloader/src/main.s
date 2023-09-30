.global pmain

.extern cmain

.section .mbhdr
.align 8
header:
	# Magic fields
	# magic
	.long 0xE85250D6
	# arch
	.long 0
	# hdr length
	.long header_end - header
	# checksum
	.long -(0xE85250D6 + 0 + (header_end - header))

	# Tags
.align 8
fb_tag:
	.short 5
	.short 0
	.long 20

	.long 1280
	.long 720
	.long 32
fb_tag_end:

.align 8
end_tag:
	# Termination tag
	.short 0
	.short 0
	.long 8
header_end:

.section .data
fault_msg:
	.asciz "Fault detected"

.align 0x1000
gdt_start:
	.quad 0
	.quad (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
gdt_end:
gdt_pointer:
	.word gdt_end - gdt_start - 1
	.quad gdt_start

.section .bss
.align 0x1000
pd1:
	.skip 0x1000
pd2:
	.skip 0x1000
pdp:
	.skip 0x1000
pml4:
	.skip 0x1000

stack:
	.skip 0x1000
stack_end:

# Paging
.section .text
.code32
pmain:
	# ha EAX értéke 0x36d76289, multiboot2 bootloader
	# által lett bootolva a rendszer
	cmpl $0x36d76289, %eax
	jne fault

	# Stack beállítása, még mielőtt
	# bármi felkerülne rá
	leal stack, %esp
	
	# AMD64 kompatibilis-e a processzor?
	movl $0x80000001, %eax
	cpuid
	and $((1 << 29)), %edx
	cmp $0, %edx
	je fault

	# Page táblák beállítása
	## %eax lesz az entry első 32 bit
	## másik 32 bit csak akkor kell ha a "fizikai" cím
	## hosszabb lesz mint 10 bit
	## Első 32 bittel az első 4 GiB-ot lehet mappelni
	# PD1 kitöltése
	mov $0, %ecx
pd1_kitolt:
	# Entry előkészítése
	movl $0x200000, %eax
	mull %ecx
	leal pd1, %edx
	# present | rw | hugepage
	orl $0b10000011, %eax

	# Entry kiírása
	movl %eax, (%edx,%ecx,8)

	incl %ecx

	# Egy táblázat 512 entry-ből áll
	cmp $512, %ecx
	jle pd1_kitolt

pdp_kitolt:
	leal pd1, %eax
	leal pdp, %edx
	# present | rw
	orl $0b11, %eax
	movl %eax, (%edx)
pml4_kitolt:
	leal pdp, %eax
	leal pml4, %edx
	# present | rw
	orl $0b11, %eax
	movl %eax, (%edx)

	# PML4 betöltése a CR3-ba
	movl %edx, %eax
	movl %eax, %cr3

	# PAE bekapcsolása
	mov %cr4, %eax
	or $0x20, %eax
	mov %eax, %cr4

	# Long mode bekapcsolása
	mov $0xc0000080, %ecx
	rdmsr
	or $0x100, %eax
	wrmsr

	# Paging bekapcsolása
	movl %cr0, %eax
	orl $0x80000000, %eax
	movl %eax, %cr0

	# GDT beállítása
	lgdt gdt_pointer

	jmp $0x08,$long_mode

.code64
long_mode:
	movw $0, %ax
	movw %ax, %ds
	movw %ax, %ss
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	call cmain

	# Jelezzük, hogy a program a végére ért
	movq $0x6969696969696969, %rax
	movq %rax, %rbx
	movq %rax, %rcx
	movq %rax, %rdx
	cli
	hlt

fault:
	# Hibakódok, így felismerem a hibát
	movl $0x01234567, %eax
	movl $0x89abcdef, %ebx
	# Processzor megállítása
halt:
	cli
	hlt
