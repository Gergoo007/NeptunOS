#pragma once

#define PAGESIZE 0x1000
#define PRELOAD_PAGESIZE 0x200000

#define _attr_packed __attribute__((packed))

#define _attr_printf //__attribute__((format(printf, 1, 2)))

#define _attr_int __attribute__ ((interrupt))
#define _attr_saved_regs __attribute__ ((no_caller_saved_registers))
