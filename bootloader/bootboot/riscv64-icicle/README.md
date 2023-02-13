BOOTBOOT MicroChip Icicle Risc-V64 Implementation
=================================================

See [BOOTBOOT Protocol](https://gitlab.com/bztsrc/bootboot) for common details.

Currently __EXPERIMENTAL__.

Generates dist/bootboot.rv64, should include Risc-V hart config and bootboot.elf and use the same format as
https://github.com/polarfire-soc/hart-software-services/tree/master/tools/hss-payload-generator

mkbootimg already knows what to do with that file, but currently installs an empty blob.
