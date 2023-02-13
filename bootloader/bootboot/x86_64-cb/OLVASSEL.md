BOOTBOOT Coreboot x86_64 Implementáció
======================================

Általános leírásért lásd a [BOOTBOOT Protokoll](https://gitlab.com/bztsrc/bootboot)t.

Ez [coreboot](https://coreboot.org) payloadként implementálja a BOOTBOOT Protokollt.
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
$ make crossgcc-i386 CPUS=$(nproc)
```

### 4. lépés - Konfigurálás

Ezután be kell konfigurálnod a coreboot-ot, hogy milyen alaplapot (vagy qemu-t) kezeljen, és hogy BOOTBOOT-al induljon.
```sh
$ make menuconfig
    válaszd a 'Mainboard' menüt
    alatta a 'Mainboard vendor' legyen '(Emulation)'
    alatta a 'Mainboard model' legyen 'QEMU x86 i440fx/piix4'
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

Bővebb információért lásd a [coreboot dokumentáció](https://doc.coreboot.org/mainboard/emulation/qemu-i440fx.html)t. Az
[images](https://gitlab.com/bztsrc/bootboot/tree/binaries/images) mappában találsz lefordított coreboot-x86.rom binárist.
```sh
$ qemu-system-x86_64 -bios build/coreboot.rom -drive file=$(BOOTBOOT)/images/disk-x86.img,format=raw -serial stdio
```

Initrd hozzáadása a ROM-hoz
---------------------------

Ahhoz, hogy bekerülhessen a ROM-ba, először is generálni kell egy initrd-t. Ez lehet (opcionálisan gzippelt) tar, cpio, stb.
archívum. Ugyancsak létrehozható az [mkbootimg](https://gitlab.com/bztsrc/bootboot/tree/master/mkbootimg) alkalmazással is:
```sh
$ ./mkbootimg myos.json initrd.bin
```
Ezután a coreboot repójában található `cbfstool` alkalmazással lehet az initrd lemezképet a ROM képhez hozzáadni:
```sh
$ ./build/cbfstool build/coreboot.rom add -t raw -f $(BOOTBOOT)/initrd.bin -n bootboot/initrd
```
Ehhez hasonlóan hozzá lehet adni egy alapértelmezett környezeti fájlt is (csak akkor használja, ha a szokásos helyeken nem
találta):
```sh
$ ./build/cbfstool build/coreboot.rom add -t raw -f environment.txt -n bootboot/config
```
Ez természetesen csak akkor működik, ha a libpayload `CONFIG_LP_CBFS=y` opcióval lett fordítva. Ennek hiányában az initrd
egy "INITRD" nevű Flashmap partíción is elhelyezhető (bár az fmaptool és az fmd formátum kezelése nem kicsit pilótavizsgás).

Gép állapot
-----------

IRQ-k letiltva, GDT nincs meghatározva, de érvényes, IDT nincs beállítva. SSE, SMP engedélyezve. Kód felügyeleti módban, 0-ás gyűrűn
fut minden processzormagon.

Telepítés
---------

Miután lefordítottad a coreboot ROM-ot BOOTBOOT payloaddal, [flashelheted a build/coreboot.rom](https://doc.coreboot.org/flash_tutorial/index.html)
fájlt az alaplapodra.

Limitációk
----------

 - A CMOS nvram nem tárol időzónát, ezért mindig GMT+0 kerül a bootboot.timezone-ba.
 - Coreboot-ban nem lehet felbontást váltani, ezért a "screen=" opciót nem kezeli.
 - Csak a SHA-XOR-CBC titkosítást ismeri, nincs AES
