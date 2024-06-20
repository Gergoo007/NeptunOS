#pragma once

#include <lib/types.h>
#include <loader/elf.h>
#include <graphics/console.h>

// Visszaadja az entry pointot
void* loader_static(void* elf);
void loader_run(void* entry);
