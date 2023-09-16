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

.section .text
pmain:
	jmp pmain
