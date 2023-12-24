.global gdt_load

gdt_load:
	lgdt (%rdi)

	// Nem lehet far jumpolni, ezért egy far return kell ide:
	// Szegmens-regiszterek értékeit beállítgatom,
	// a %cs-t azonban far returnnel kell, ezért van a
	// stacken, hogy majd a retfq leszedje és beállítsa
	// Azért 0x10, mert a GDT-ben a kdata offset 0x10-nél kezdődik
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	pop %rdi
	// kcode pedig a knull miatt 0x08-nál kezdődik
	mov $0x08, %rax
	push %rax
	push %rdi
	retfq
