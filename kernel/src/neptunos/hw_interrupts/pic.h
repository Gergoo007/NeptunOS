#pragma once

#include <neptunos/libk/stdint.h>

#include "../serial/com.h"

// Shamelessly stolen from https://wiki.osdev.org/PIC

#define PIC_M			0x20
#define PIC_M_COMMAND	PIC_M
#define PIC_M_DATA		(PIC_M+1)

#define PIC_S			0xA0
#define PIC_S_COMMAND	PIC_S
#define PIC_S_DATA		(PIC_S+1)

// EOI stands for End Of Interrupt
#define PIC_EOI 		0x20

#define ICW1_ICW4		0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE		0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL		0x08		/* Level triggered (edge) mode */
#define ICW1_INIT		0x10		/* Initialization - required! */
 
#define ICW4_8086		0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO		0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM		0x10		/* Special fully nested (not) */

void remap_pic(uint8_t offset1, uint8_t offset2);

static inline void io_wait() { //asm volatile ("outb %0, $0x80"); }
}

static inline void _out8(uint16_t port, uint8_t data) {
    out8(port, data);
	io_wait();	
}

static inline uint8_t _in8(uint16_t port) {
	uint8_t ret = in8(port);
	io_wait();	
	return ret;
}
