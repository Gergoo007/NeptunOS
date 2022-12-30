#pragma once

struct interrupt_frame;
__attribute__((interrupt)) void page_flt_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void double_flt_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void invalid_opcode_flt_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void general_protection_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void custom_handler(struct interrupt_frame* frame);