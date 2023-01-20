#pragma once

// TODO: translate all this into asm

#include <neptunos/libk/stdall.h>
#include <neptunos/config/attributes.h>

// static inline uint8_t in8(uint16_t port) {
// 	uint8_t data;
// 	asm volatile("inb %1, %0"
// 				: "=a"(data)
// 				: "d"(port));
// 	return data;
// }

extern uint8_t inb(uint16_t);
extern uint8_t inw(uint16_t);
extern uint8_t inl(uint16_t);

extern void outb(uint8_t data, uint16_t port);
extern void outw(uint16_t data, uint16_t port);
extern void outl(uint32_t data, uint16_t port);

extern void io_wait();

static _attr_no_caller_saved_regs void _in8(uint16_t port, uint8_t* data) {
	*data = inb(port);
}
