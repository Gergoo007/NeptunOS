.global idt_load

idt_load:
	lidt (%rdi)
	ret
