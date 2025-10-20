#pragma once

#include <util/elf.hh>

extern Elf64_Sym* ksyms;
extern u32 num_ksyms;

extern char* strtab;
extern char* shstrtab;

void ksyms_read();
