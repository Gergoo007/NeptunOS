#pragma once

#include <neptunos/graphics/graphics.h>
#include <neptunos/graphics/text_renderer.h>

#define panic(msg, ...) _panic((char*)msg, (char*)__func__, (char*)__FILE__, __LINE__, ##__VA_ARGS__)

_attr_no_caller_saved_regs void _panic(char msg[256], char* caller, char* file, uint32_t line, ...);
