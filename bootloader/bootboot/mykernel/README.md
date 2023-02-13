BOOTBOOT Example Kernels
========================

See [BOOTBOOT Protocol](https://gitlab.com/bztsrc/bootboot) for common details.

These are sample "Hello World" kernels written in different system languages. Binary
versions [here](https://gitlab.com/bztsrc/bootboot/tree/binaries/mykernel). They all
should output a screen like this after a successful boot:

<img src="https://gitlab.com/bztsrc/bootboot/raw/binaries/mykernel/screenshot.png" alt="Screenshot">

Compilation
-----------

In the language's directory, just run `make`. You'll need `gcc`, `g++`, `gnat` (GNU Ada), `fpc` (FreePascal
Compiler), `cargo` + `rust`, and `gccgo` (GNU go-lang compiler, NOT the official go-lang compiler!).
