/*
 * aarch64-cb/bootboot.c
 *
 * Copyright (C) 2017 - 2021 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the BOOTBOOT Protocol package.
 * @brief Boot loader for the AArch64 coreboot payload
 *
 */

#define BBDEBUG 1

/*

unfortunately currently you cannot compile libpayload for ARM64. It gives this error:

$ cd libpayload
$ make
    CC         arch/arm64/main.libc.o
<command-line>: error: "_LP64" redefined [-Werror]
<built-in>: note: this is the location of the previous definition
cc1: error: unrecognized command line option '-Wno-address-of-packed-member' [-Werror]
cc1: all warnings being treated as errors
make: *** [Makefile:270: build/arch/arm64/main.libc.o] Error 1
*/

#include <libpayload-config.h>
#include <libpayload.h>

extern struct sysinfo_t lib_sysinfo;

int main(void)
{
    int ret, i;

    printf("Hello World!\n");

    ret = lib_get_sysinfo();
    if (ret) halt();

    printf("Number of memory ranges: %d\n", lib_sysinfo.n_memranges);
    for (i = 0; i < lib_sysinfo.n_memranges; i++) {
        printf("%d: base 0x%08llx size 0x%08llx type 0x%x\n", i, lib_sysinfo.memrange[i].base, lib_sysinfo.memrange[i].size, lib_sysinfo.memrange[i].type);
    }
    printf("Now we will halt. Bye\n");
    halt();
    return 0;
}
