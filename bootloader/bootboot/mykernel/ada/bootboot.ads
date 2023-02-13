--
-- mykernel/ada/bootboot.ads
--
-- Copyright (C) 2017 - 2021 bzt (bztsrc@gitlab)
--
-- Permission is hereby granted, free of charge, to any person
-- obtaining a copy of this software and associated documentation
-- files (the "Software"), to deal in the Software without
-- restriction, including without limitation the rights to use, copy,
-- modify, merge, publish, distribute, sublicense, and/or sell copies
-- of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
-- NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
-- HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
-- WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
-- DEALINGS IN THE SOFTWARE.
--
-- This file is part of the BOOTBOOT Protocol package.
-- @brief A sample BOOTBOOT compatible kernel
--
--

with System;

package bootboot is
    type UInt8 is mod (2 ** 8);
    type UInt16 is mod (2 ** 16);
    type UInt32 is mod (2 ** 32);
    type UInt64 is mod (2 ** 64);
    type magic_type is array(0 .. 3) of UInt8;
    type timezone_type is range -1440 .. +1440;
    type datetime_type is array(0 .. 7) of UInt8;
    type Screen is array(0 .. Integer'Last) of Uint32;

    BOOTBOOT_MMIO : constant := 16#ffffffff_f8000000#; -- memory mapped IO virtual address
    BOOTBOOT_FB : constant :=   16#ffffffff_fc000000#; -- frame buffer virtual address
    BOOTBOOT_INFO : constant := 16#ffffffff_ffe00000#; -- bootboot struct virtual address
    BOOTBOOT_ENV : constant :=  16#ffffffff_ffe01000#; -- environment string virtual address
    BOOTBOOT_CORE : constant := 16#ffffffff_ffe02000#; -- core loadable segment start

    -- minimum protocol level:
    --   hardcoded kernel name, static kernel memory addresses
    PROTOCOL_MINIMAL : constant := 0;
    -- static protocol level:
    --   kernel name parsed from environment, static kernel memory addresses
    PROTOCOL_STATIC  : constant := 1;
    -- dynamic protocol level:
    --   kernel name parsed, kernel memory addresses from ELF or PE symbols
    PROTOCOL_DYNAMIC  : constant := 2;
    -- big-endian flag
    PROTOCOL_BIGENDIAN : constant := 128;

    -- loader types, just informational
    LOADER_BIOS     : constant := 0;
    LOADER_UEFI     : constant := 2;
    LOADER_RPI      : constant := 4;
    LOADER_COREBOOT : constant := 8;

    -- framebuffer pixel format, only 32 bits supported
    FB_ARGB : constant := 0;
    FB_RGBA : constant := 1;
    FB_ABGR : constant := 2;
    FB_BGRA : constant := 3;

    MMAP_USED   : constant := 0;  -- don't use. Reserved or unknown regions
    MMAP_FREE   : constant := 1;  -- usable memory
    MMAP_ACPI   : constant := 2;  -- acpi memory, volatile and non-volatile as well
    MMAP_MMIO   : constant := 3;  -- memory mapped IO region

    INITRD_MAXSIZE : constant := 16; -- Mb

    fb : Screen;
    for fb'Address use System'To_Address(BOOTBOOT_FB);
    pragma Volatile (fb);

    type bootboot_struct is
        record
            -- first 64 bytes is platform independent
            magic : magic_type;         -- 'BOOT' magic
            size : UInt32;              -- length of bootboot structure, minimum 128
            protocol : UInt8;           -- 1, static addresses, see PROTOCOL_* and LOADER_* above
            fb_type : UInt8;            -- framebuffer type, see FB_* above
            numcores : UInt16;          -- number of processor cores
            bspid : UInt16;             -- Bootsrap processor ID (Local APIC Id on x86_64)
            timezone : timezone_type;   -- in minutes -1440..1440
            datetime : datetime_type;   -- in BCD yyyymmddhhiiss UTC (independent to timezone)
            initrd_ptr : UInt64;        -- ramdisk image position and size
            initrd_size : UInt64;
            fb_ptr : UInt64;            -- framebuffer pointer and dimensions
            fb_size : UInt32;
            fb_width : UInt32;
            fb_height : UInt32;
            fb_scanline : UInt32;
            -- the rest (64 bytes) is platform specific
            x86_64_acpi_ptr : UInt64;
            x86_64_smbi_ptr : UInt64;
            x86_64_efi_ptr : UInt64;
            x86_64_mp_ptr : UInt64;
            x86_64_unused0 : UInt64;
            x86_64_unused1 : UInt64;
            x86_64_unused2 : UInt64;
            x86_64_unused3 : UInt64;
        end record;

    bootboot : bootboot_struct;
    for bootboot'Address use System'To_Address(BOOTBOOT_INFO);
    pragma Volatile (bootboot);

    -- mmap entry, type is stored in least significant tetrad (half byte) of size
    -- this means size described in 16 byte units (not a problem, most modern
    -- firmware report memory in pages, 4096 byte units anyway).
    type MMapEnt_type is
        record
            ptr : UInt64;
            size : UInt64;
        end record;

    MMap : MMapEnt_type;
    for MMap'Address use System'To_Address(BOOTBOOT_INFO + 128);
    pragma Volatile (MMap);

end bootboot;
