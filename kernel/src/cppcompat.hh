#pragma once

extern void (*__init_array[])();
extern void (*__init_array_end[])();

void cpp_construct_objects();
