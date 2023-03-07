;
;	Credit: Philipp Oppermann (https://os.phil-opp.com/edition-1/)
;

; PML4 is not required as we are using 2 MiB pages

global onboot
extern c_main

section .realtext
h:
	dw 0xffff

section .text
bits 32
onboot:
	jmp onboot
	mov esp, stack_top

	call set_up_page_tables
	call enable_paging

	; load the 64-bit GDT
	lgdt [gdt64.pointer]

	mov edi, ebx
	jmp gdt64.code:c_main
	
	hlt

set_up_page_tables:
	; PD -> PDP
	mov eax, pd_table
	or eax, 0b11 ; present + writable
	mov [pdp_table], eax

	; PT -> PD
	mov eax, pt_table
	or eax, 0b11 ; present + writable
	mov [pd_table], eax

	; map each PT entry to a huge 2MiB page
	mov ecx, 0         ; counter variable

.map_pt_table:
	; map ecx-th PT entry to a huge page that starts at address 2MiB*ecx
	mov eax, 0x200000  ; 2MiB
	mul ecx            ; start address of ecx-th page
	or eax, 0b10000011 ; present + writable + huge
	mov [pt_table + ecx * 8], eax ; map ecx-th entry

	inc ecx            ; increase counter
	cmp ecx, 512       ; if counter == 512, the whole PD table is mapped
	jne .map_pt_table  ; else map the next entry

	ret

enable_paging:
	; load PDP to cr3 register (cpu uses this to access the PDP table)
	mov eax, pdp_table
	mov cr3, eax

	; enable PAE-flag in cr4 (Physical Address Extension)
	mov eax, cr4
	or eax, 1 << 5
	mov cr4, eax

	; set the long mode bit in the EFER MSR (model specific register)
	mov ecx, 0xC0000080
	rdmsr
	or eax, 1 << 8
	wrmsr

	; enable paging in the cr0 register
	mov eax, cr0
	or eax, 1 << 31
	mov cr0, eax

	ret

section .bss
align 4096
pdp_table:
	resb 4096
pd_table:
	resb 4096
pt_table:
	resb 4096

stack_bottom:
	resb 64
stack_top:

section .rodata
gdt64:
	dq 0 ; zero entry
.code: equ $ - gdt64 ; new
	dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; code segment
.pointer:
	dw $ - gdt64 - 1
	dq gdt64
