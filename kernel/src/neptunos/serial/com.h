#pragma once

// TODO: translate all this into asm

#include <neptunos/libk/stdint.h>
#include <neptunos/config/attributes.h>

static inline uint8_t in8(uint16_t port) {
    uint8_t data;
    asm volatile("inb %1, %0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}

static inline uint16_t in16(uint16_t port) {
    uint16_t data;
    asm volatile("inw %1, %0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}

static inline uint32_t in32(uint16_t port) {
    uint32_t data;
    asm volatile("inl %1, %0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}

static inline void out8(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1"
                 :
                 : "a"(data), "d"(port));
}

static inline void out16(uint16_t port, uint16_t data) {
    asm volatile("outw %0, %1"
                 :
                 : "a"(data), "d"(port));
}

static inline void out32(uint16_t port, uint32_t data) {
    asm volatile("outl %0, %1"
                 :
                 : "a"(data), "d"(port));
}

static inline void io_wait() {
    asm volatile("outb %%al, $0x80" :: "a"(0));
}

static inline void _out8(uint16_t port, uint8_t data) {
	out8(port, data);
	io_wait();	
}

_attr_no_caller_saved_regs static inline void _in8(uint16_t port, uint8_t* data) {
	*data = in8(port);
	io_wait();
}
