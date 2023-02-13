BOOTBOOT Minta Kernelek
=======================

Általános leírásért lásd a [BOOTBOOT Protokoll](https://gitlab.com/bztsrc/bootboot)t.

Ezek példa "Hello Világ" kernelek különböző programozási nyelveken írva. Bináris verzió
elérhető [itt](https://gitlab.com/bztsrc/bootboot/tree/binaries/mykernel). Sikeres indítás
után egy ilyen képernyőt kell látnod:

<img src="https://gitlab.com/bztsrc/bootboot/raw/binaries/mykernel/screenshot.png" alt="Screenshot">

Fordítás
--------

Az adott nyelv mappájában csak futtasd a `make` parancsot. Kelleni fog a `gcc`, `g++`, `gnat` (GNU Ada),
`fpc` (FreePascal Compiler), `cargo` + `rust`, és a `gccgo` (GNU go-lang fordító, NEM a hivatalos go-lang fordító!).

