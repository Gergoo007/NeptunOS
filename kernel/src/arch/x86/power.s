.global halt
.global clearint
.global setint

.code64
.section .text
halt:
	hlt
	ret

clearint:
	cli
	ret

setint:
	sti
	ret
