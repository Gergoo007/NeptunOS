#pragma once

#include <types.hh>

static inline void outb(u16 port, u8 data) {
	asm volatile ("outb %0, %1" :: "a"((u8)data), "d"((u16)port));
}

static inline void outw(u16 port, u16 data) {
	asm volatile ("outw %0, %1" :: "a"((u16)data), "d"((u16)port));
}

static inline void outl(u16 port, u32 data) {
	asm volatile ("outl %0, %1" :: "a"((u32)data), "d"((u16)port));
}

static inline u8 inb(u16 port) {
	u8 data;
	asm volatile ("inb %%dx, %%al" : "=a"(data) : "d"((u16)port));
	return data;
}

static inline u16 inw(u16 port) {
	u16 data;
	asm volatile ("inw %%dx, %%ax" : "=a"(data) : "d"((u16)port));
	return data;
}

static inline u32 inl(u16 port) {
	u32 data;
	asm volatile ("inl %%dx, %%eax" : "=a"(data) : "d"((u16)port));
	return data;
}
