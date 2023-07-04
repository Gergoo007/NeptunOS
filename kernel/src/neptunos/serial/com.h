#pragma once

#include <neptunos/libk/stdall.h>
#include <neptunos/config/attributes.h>

extern uint8_t inb(uint16_t);
extern uint8_t inw(uint16_t);
extern uint8_t inl(uint16_t);

extern void outb(uint8_t data, uint16_t port);
extern void outw(uint16_t data, uint16_t port);
extern void outl(uint32_t data, uint16_t port);

extern void io_wait(u32 rounds);

_attr_no_caller_saved_regs void _in8(uint16_t port, uint8_t* data);
