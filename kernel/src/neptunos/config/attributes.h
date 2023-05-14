#pragma once

// Configure attributes for your compiler
// Default is GNU C attributes
#define _attr_packed __attribute__((packed))
#define _attr_int __attribute__((interrupt))
#define _attr_no_caller_saved_regs __attribute__((no_caller_saved_registers))
