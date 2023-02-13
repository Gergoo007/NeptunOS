BOOTBOOT Coreboot x86_64 Implementation
=======================================

See [BOOTBOOT Protocol](https://gitlab.com/bztsrc/bootboot) for common details.

Implements the BOOTBOOT Protocol as a [coreboot](https://coreboot.org) payload.
Must be compiled using the coreboot build environment.

Compilation
-----------

### Step 1 - Install dependencies

First, install the [coreboot dependencies](https://doc.coreboot.org/tutorial/part1.html): bison, build-essentials, curl,
flex, git, gnat, libncurses5-dev, m4, zlib. Please refer to the linked doc for up-to-date list of dependencies.

### Step 2 - Get coreboot

Download coreboot and its submodules
```sh
$ git clone https://review.coreboot.org/coreboot
$ cd coreboot
$ git submodule update --init
```
It is very important to initialize submodules, otherwise you won't be able to compile coreboot.

### Step 3 - Create toolchain

This step could take a while. Replace *$(nproc)* with the number of CPU cores you have.
```sh
$ make crossgcc-i386 CPUS=$(nproc)
```

### Step 4 - Configure

Now configure coreboot for your motherboard (or qemu) and BOOTBOOT.
```sh
$ make menuconfig
    select 'Mainboard' menu
    Beside 'Mainboard vendor' should be '(Emulation)'
    Beside 'Mainboard model' should be 'QEMU x86 i440fx/piix4'
    select 'Exit'
    select 'Devices' menu
    select 'Display' menu
    Beside 'Framebuffer mode' should be 'Linear "high-resolution" framebuffer'
    select 'Exit'
    select 'Exit'
    select 'Payload' menu
    select 'Add a Payload'
    choose 'BOOTBOOT'
    select 'Exit'
    select 'Exit'
    select 'Yes'
```
It is important to set the display to "linear framebuffer", because BOOTBOOT does not handle the legacy, non-portable VGA
text mode. Sadly there's no way of configuring this in run-time with libpayload.

### Step 5 - Build coreboot

```sh
$ make
```

### Step 6 - Test the newly compiled ROM in QEMU

For more information, read the [coreboot docs](https://doc.coreboot.org/mainboard/emulation/qemu-i440fx.html). In the
[images](https://gitlab.com/bztsrc/bootboot/tree/binaries/images) directory you can find a precompiled coreboot-x86.rom binary.
```sh
$ qemu-system-x86_64 -bios build/coreboot.rom -drive file=$(BOOTBOOT)/images/disk-x86.img,format=raw -serial stdio
```

Adding Initrd to ROM
--------------------

To add an initrd into ROM, first you have to generate one. It can be an (optionally gzipped) tar, cpio, etc. archive. You can
also create it using the [mkbootimg](https://gitlab.com/bztsrc/bootboot/tree/master/mkbootimg) utility:
```sh
$ ./mkbootimg myos.json initrd.bin
```
Then use the `cbfstool` utility in the coreboot repository to add the initrd image into the ROM image:
```sh
$ ./build/cbfstool build/coreboot.rom add -t raw -f $(BOOTBOOT)/initrd.bin -n bootboot/initrd
```
You can add a fallback environment configuration similarily (only used if environment cannot be loaded from the usual places):
```sh
$ ./build/cbfstool build/coreboot.rom add -t raw -f $(BOOTBOOT)/environment.txt -n bootboot/config
```
Obviously this can only work if libpayload was compiled with `CONFIG_LP_CBFS=y`. Without you can still place the initrd on
a Flashmap partition named "INITRD" (however using the fmaptool and dealing with the fmd format is a rocket science).

Machine state
-------------

IRQs masked. GDT unspecified, but valid, IDT unset. SSE, SMP enabled. Code is running in supervisor mode in ring 0 on all cores.

Installation
------------

Once you have compiled a coreboot ROM with the BOOTBOOT payload, you can
[flash the build/coreboot.rom](https://doc.coreboot.org/flash_tutorial/index.html) file to your mainboard.

Limitations
-----------

 - The CMOS nvram does not store timezone, so always GMT+0 returned in bootboot.timezone.
 - Coreboot does not provide a way to set screen resolution, so "screen=" config option is not used.
 - Only supports SHA-XOR-CBC, no AES
