/*
 * x86_64-cb/smp.S
 *
 * Copyright (C) 2017 - 2021 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the BOOTBOOT Protocol package.
 * @brief SMP and long mode initialization code.
 *
 */

.globl ap_trampoline
.globl bsp_init
.globl bsp64_init
.extern lapic_ids
.extern lapic_addr
.extern initstack

    .text

/*****************************************************************************
 * things to do on the APs                                                   *
 *****************************************************************************/
    .balign 128
    .code16
    /* this code will be relocated to 0x1000 - 0x1100 */
ap_trampoline:
    cli
    cld
    ljmp    $0, $0x1040
    .balign 16
    // prot mode GDT
_L1010_GDT_table:
    .long 0, 0
    .long 0x0000FFFF, 0x00CF9A00    // flat code
    .long 0x0000FFFF, 0x008F9200    // flat data
    .long 0x00000068, 0x00CF8900    // tss, not used but required by VB's vt-x
_L1030_GDT_value:
    .word _L1030_GDT_value - _L1010_GDT_table - 1
    .long 0x1010
    .long 0, 0
    .balign 64
_L1040:
    xorw    %ax, %ax
    movw    %ax, %ds
    lgdtl   0x1030
    movl    %cr0, %eax
    orl     $1, %eax
    movl    %eax, %cr0
    ljmp    $8, $0x1060
    .balign 32
    .code32
_L1060:
    movw    $16, %ax
    movw    %ax, %ds
    incb    0x1011
    // spinlock until BSP finishes
1:  pause
    cmpb    $0, 0x1010
    jz      1b
    // jump back to non-relocated code segment
    ljmp    $8, $longmode_init
    .balign 128
ap_trampoline_end:

    // long mode GDT (here it is aligned and out of execution flow)
GDT_table:
    .long 0, 0
    .long 0x0000FFFF, 0x00209800    // flat code, ring 0
    .long 0x0000FFFF, 0x00809200    // flat data
    .long 0x00000068, 0x00008900    // tss, required by vt-x
    .long 0, 0
GDT_value:
    .word GDT_value - GDT_table - 1
    .long GDT_table, 0, 0
    .word 0
    .balign 8
stack64:
    .long bootboot_startcore
    .long 0
    .quad 8

/*****************************************************************************
 * things to do on BSP                                                       *
 *****************************************************************************/
    /* these are 32 bit encoded instructions */
bsp_init:
    cli
    cld
    movb    $0xFF, %al          // disable PIC
    outb    %al, $0x21
    outb    %al, $0xA1
    inb     $0x70, %al          // disable NMI
    orb     $0x80, %al
    outb    %al, $0x70
    incb    0x1010              // release AP spin lock
    // fall into long mode initialization code

/*****************************************************************************
 * common code for all cores, enable long mode and start kernel              *
 *****************************************************************************/
longmode_init:
    // enable lapic and find our lapic id
    movl    lapic_addr, %edi
    or      %edi, %edi
    jz      1f
    addl    $0xF0, %edi
    movl    (%edi), %eax
    or      $0x1, %ah
    movl    %eax, (%edi)
    subl    $0xD0, %edi
    movl    (%edi), %edi
    shrl    $24, %edi
1:  // do not clobber di

    movl    $0x368, %eax        // Set PAE, MCE, PGE; OSFXSR, OSXMMEXCPT (enable SSE)
    movl    %eax, %cr4
    movl    $0x4000, %eax
    movl    %eax, %cr3
    movl    $0x0C0000080, %ecx  // EFR MSR
    rdmsr
    orl     $0x100, %eax        // enable long mode
    wrmsr

    movl    $0x0C0000011, %eax  // clear EM, MP (enable SSE) and WP
    movl    %eax, %cr0
    lgdt    GDT_value
    ljmp    $8, $bootboot_startcore

    .code64
    /* similar code to above, but these are 64 bit encoded, only needed on BSP if coreboot is compiled for x86_64 */
bsp64_init:
    // do not clobber di
    cli
    cld
    movb    $0xFF, %al          // disable PIC
    outb    %al, $0x21
    outb    %al, $0xA1
    inb     $0x70, %al          // disable NMI
    orb     $0x80, %al
    outb    %al, $0x70
    incb    0x1010              // release AP spin lock

    xorq    %rax, %rax
    movl    $0xC0000011, %eax   // enable SSE
    movq    %rax, %cr0
    movq    %cr4, %rax
    orw     $3 << 8, %ax
    mov     %rax, %cr4
    movl    $0x4000, %eax       // set up paging
    movq    %rax, %cr3
    xorq    %rax, %rax
    movl    $GDT_value, %eax
    lgdt    (%rax)
    movl    $stack64, %eax      // reload CS, that's tricky in long mode because ljmp doesn't work
    movq    %rax, %rsp
    lretq

    /* IN: di = apic id of current core */
bootboot_startcore:
    movl    $0x10, %eax         // load long mode segments
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
    movw    %ax, %fs
    movw    %ax, %gs
    movzwq  %di, %rbx
    shll    $1, %ebx            // ebx = lapic id * 2
    addl    $lapic_ids, %ebx
    xorq    %rax, %rax
    movw    (%rbx), %ax         // ax = word[lapic_ids + lapic id * 2]
    movl    $initstack, %ebx
    movl    (%rbx), %ebx
    movzwq  %ax, %rdi
    mulq    %rbx                // 1k stack for each core

    // set stack and call _start() in sys/core
    xorq    %rsp, %rsp          // sp = core_num * -initstack
    subq    %rax, %rsp
    xorq    %rsi, %rsi
    movl    $entrypoint, %esi   // GAS does not allow "jmp qword[entrypoint]"
    lodsq
    jmp     *%rax
    hlt
