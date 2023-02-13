BOOTBOOT Coreboot ARM64 Implementation
======================================

See [BOOTBOOT Protocol](https://gitlab.com/bztsrc/bootboot) for common details.

Implements the BOOTBOOT Protocol as a [coreboot](https://coreboot.org) payload. Currently __EXPERIMENTAL__.
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
$ make crossgcc-aarch64 CPUS=$(nproc)
```

### Step 4 - Configure

Now configure coreboot for your motherboard (or qemu) and BOOTBOOT.
```sh
$ make menuconfig
    select 'Mainboard' menu
    Beside 'Mainboard vendor' should be '(Emulation)'
    Beside 'Mainboard model' should be 'QEMU AArch64'
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

For more information, read [coreboot docs](https://doc.coreboot.org/mainboard/emulation/qemu-aarch64.html).
```sh
$ qemu-system-aarch64 -bios build/coreboot.rom -M virt,secure=on,virtualization=on -cpu cortex-a53 -m 1024M \
    -drive file=$(BOOTBOOT)/images/disk-rpi.img,format=raw -serial stdio
```

Machine state
-------------

Code is running in supervisor mode, at EL1 on all cores.

Installation
------------

Once you have compiled a coreboot ROM with the BOOTBOOT payload, you can
[flash the build/coreboot.rom](https://doc.coreboot.org/flash_tutorial/index.html) file to your mainboard.

Limitations
-----------

 - Only supports SHA-XOR-CBC, no AES
