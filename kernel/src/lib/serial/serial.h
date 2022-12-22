#pragma once

#include <stdint.h>

extern void serial_init();

extern void serial_write(uint16_t port, char* msg);
extern void serial_write_c(uint16_t port, char msg);
extern void serial_write_d(uint16_t port, int msg);
