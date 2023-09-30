.global pmain

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

# Paging
.section .text
.code32
pmain:
	# ha EAX értéke 0x36d76289, multiboot2 bootloader
	# által lett bootolva a rendszer
	cmpl $0x36d76289, %eax
	jne fault
	
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
	orl $0b11000001, %eax

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

	movl $0x20, %eax
	movl $0x10, %ebx
	add %ebx, %eax

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

	cli
	hlt

.code64
long_mode:
	jmp .

fault:
	# Hibakódok, így felismerem a hibát
	movl $0x01234567, %eax
	movl $0x89abcdef, %ebx
	# Processzor megállítása
halt:
	cli
	hlt
