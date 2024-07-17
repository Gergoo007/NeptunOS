.global memset
.global memcpy

# %rdi: cím
# %sl: érték
# %rdx: ennyit
memset:
	movq %rdx, %rcx
	rep stosb
	ret

# %rdi: innen
# %rsi: amoda
# %rdx: ennyit
memcpy:
	xchg %rdi, %rsi
	movq %rdx, %rcx
	rep movsb
	ret
