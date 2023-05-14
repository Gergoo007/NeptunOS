#pragma once

#include <neptunos/graphics/panic.h>
#include <neptunos/memory/memory.h>

#define assert(x, msg, ...) if(x) _panic((char*)msg, (char*)__func__, (char*)__FILE__, __LINE__, ##__VA_ARGS__);
