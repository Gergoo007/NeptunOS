#pragma once

#include <lib/int.h>

u8 inb(u16);
u8 inw(u16);
u8 inl(u16);

void outb(u8 data, u16 port);
void outw(u16 data, u16 port);
void outl(u32 data, u16 port);