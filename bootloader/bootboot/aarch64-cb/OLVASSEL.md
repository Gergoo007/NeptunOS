BOOTBOOT Coreboot ARM64 Implementáció
=====================================

Általános leírásért lásd a [BOOTBOOT Protokoll](https://gitlab.com/bztsrc/bootboot)t.

Ez [coreboot](https://coreboot.org) payloadként implementálja a BOOTBOOT Protokollt. Jelenleg __FEJLESZTÉS ALATT__.
A coreboot fordítási környezetben fordítandó.

Fordítás
--------

### 1. lépés - Függőségek telepítése

Először is, telepíteni kell a [coreboot függőségei](https://doc.coreboot.org/tutorial/part1.html)t: bison, build-essentials, curl,
flex, git, gnat, libncurses5-dev, m4, zlib. A legfrissebb listáért nézd meg a linkelt dokumentumot.

### 2. lépés - A coreboot beszerzése

Töltsd le a coreboot-ot minden almoduljával együtt
```sh
$ git clone https://review.coreboot.org/coreboot
$ cd coreboot
$ git submodule update --init
```
Nagyon fontos, hogy inicializáld az almodulokat, máskülönben nem fog lefordulni a coreboot.

### 3. lépés - Fordítókörnyezet (toolchain) létrehozása

Ez el fog tartani egy jódarabig. A *$(nproc)* helyett add meg, hogy hány CPU core-od van.
```sh
$ make crossgcc-aarch64 CPUS=$(nproc)
```

### 4. lépés - Konfigurálás

Ezután be kell konfigurálnod a coreboot-ot, hogy milyen alaplapot (vagy qemu-t) kezeljen, és hogy BOOTBOOT-al induljon.
```sh
$ make menuconfig
    válaszd a 'Mainboard' menüt
    alatta a 'Mainboard vendor' legyen '(Emulation)'
    alatta a 'Mainboard model' legyen 'QEMU AArch64'
    válaszd az 'Exit'-t
    válaszd a 'Devices' menüt
    válaszd a 'Display' menüt
    alatta a 'Framebuffer mode' legyen 'Linear "high-resolution" framebuffer'
    válaszd az 'Exit'-t
    válaszd az 'Exit'-t
    válaszd a 'Payload' menüt
    válaszd az 'Add a Payload' opciót
    válaszd ki a 'BOOTBOOT'-ot
    válaszd az 'Exit'-t
    válaszd az 'Exit'-t
    válaszd a 'Yes'-t
```
Fontos, hogy a kijelzőt "linear framebuffer"-re állítsd, mert a BOOTBOOT nem kezeli az elavult, nem portolható VGA szöveges
módot. Sajnos a libpayload nem támogatja a futás időben való beállítást.

### 5. lépés - A coreboot fordítása

```sh
$ make
```

### 6. lépés - A frissen fordított ROM tesztelése QEMU-n

Bővebb információért lásd a [coreboot dokumentáció](https://doc.coreboot.org/mainboard/emulation/qemu-aarch64.html)t.
```sh
$ qemu-system-aarch64 -bios build/coreboot.rom -M virt,secure=on,virtualization=on -cpu cortex-a53 -m 1024M \
    -drive file=$(BOOTBOOT)/images/disk-rpi.img,format=raw -serial stdio
```

Gép állapot
-----------

A kód felügyeleti módban, EL1-en fut minden processzoron.

Telepítés
---------

Miután lefordítottad a coreboot ROM-ot BOOTBOOT payloaddal, [flashelheted a build/coreboot.rom](https://doc.coreboot.org/flash_tutorial/index.html)
fájlt az alaplapodra.

Limitációk
----------

 - Csak a SHA-XOR-CBC titkosítást ismeri, nincs AES
