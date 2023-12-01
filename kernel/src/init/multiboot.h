#pragma once

#include <lib/multiboot.h>
#include <serial/serial.h>
#include <graphics/graphics.h>
#include <memory/heap/pmm.h>
#include <acpi/acpi.h>

void mb_parse_tags(u32 hdr);
