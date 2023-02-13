{*
 * mykernel/pas/bootboot.inc
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
 * @brief A sample BOOTBOOT compatible kernel
 *
 *}

Const
    BOOTBOOT_MMIO = $fffffffff8000000;  { memory mapped IO virtual address }
    BOOTBOOT_FB   = $fffffffffc000000;  { frame buffer virtual address }
    BOOTBOOT_INFO = $ffffffffffe00000;  { bootboot struct virtual address }
    BOOTBOOT_ENV  = $ffffffffffe01000;  { environment string virtual address }
    BOOTBOOT_CORE = $ffffffffffe02000;  { core loadable segment start }

{ minimum protocol level:
   hardcoded kernel name, static kernel memory addresses }
    PROTOCOL_MINIMAL = 0;
{ static protocol level:
   kernel name parsed from environment, static kernel memory addresses }
    PROTOCOL_STATIC  = 1;
{ dynamic protocol level:
   kernel name parsed, kernel memory addresses from ELF or PE symbols }
    PROTOCOL_DYNAMIC  = 2;
{ big-endian flag }
    PROTOCOL_BIGENDIAN = $80;

{ loader types, just informational }
    LOADER_BIOS     = (0 shl 2);
    LOADER_UEFI     = (1 shl 2);
    LOADER_RPI      = (2 shl 2);
    LOADER_COREBOOT = (3 shl 2);

{ framebuffer pixel format, only 32 bits supported }
    FB_ARGB = 0;
    FB_RGBA = 1;
    FB_ABGR = 2;
    FB_BGRA = 3;

    MMAP_USED   = 0;  { don't use. Reserved or unknown regions }
    MMAP_FREE   = 1;  { usable memory }
    MMAP_ACPI   = 2;  { acpi memory, volatile and non-volatile as well }
    MMAP_MMIO   = 3;  { memory mapped IO region }

    INITRD_MAXSIZE = 16; { Mb }

Type
    Pbootboot = ^Tbootboot;
    Tbootboot = Packed Record
        { first 64 bytes is platform independent }
        magic : Array[0..3] of Byte;    { 'BOOT' magic }
        size : UInt32;                  { length of bootboot structure, minimum 128 }
        protocol : Byte;                { 1, static addresses, see PROTOCOL_* and LOADER_* above }
        fb_type : Byte;                 { framebuffer type, see FB_* above }
        numcores : UInt16;              { number of processor cores }
        bspid : UInt16;                 { Bootsrap processor ID (Local APIC Id on x86_64) }
        timezone : Int16;               { in minutes -1440..1440 }
        datetime : Array [0..7] of Byte;{ in BCD yyyymmddhhiiss UTC (independent to timezone) }
        initrd_ptr : UInt64;            { ramdisk image position and size }
        initrd_size : UInt64;
        fb_ptr : UInt64;                { framebuffer pointer and dimensions }
        fb_size : UInt32;
        fb_width : UInt32;
        fb_height : UInt32;
        fb_scanline : UInt32;
        { the rest (64 bytes) is platform specific }
{$ifdef CPUX86_64}
        x86_64_acpi_ptr : UInt64;
        x86_64_smbi_ptr : UInt64;
        x86_64_efi_ptr : UInt64;
        x86_64_mp_ptr : UInt64;
        x86_64_unused0 : UInt64;
        x86_64_unused1 : UInt64;
        x86_64_unused2 : UInt64;
        x86_64_unused3 : UInt64;
{$else}
        aarch64_acpi_ptr : UInt64;
        aarch64_mmio_ptr : UInt64;
        aarch64_efi_ptr : UInt64;
        aarch64_unused0 : UInt64;
        aarch64_unused1 : UInt64;
        aarch64_unused2 : UInt64;
        aarch64_unused3 : UInt64;
        aarch64_unused4 : UInt64;
{$endif}
    end;

{ mmap entry, type is stored in least significant tetrad (half byte) of size
  this means size described in 16 byte units (not a problem, most modern
  firmware report memory in pages, 4096 byte units anyway). }
    PMMapEnt = ^TMMapEnt;
    TMMapEnt = Packed Record
        ptr : QWord;
        size : QWord;
    End;

    MMap = PMMapEnt;
    { MMap = PMMapEnt(@bootboot + 128); }
