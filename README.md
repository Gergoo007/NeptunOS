# NeptunOS
is my attempt at a hobby operating system. This is my second bigger project (the first being the 6502 emulator), and my first OS. It is always Work In Progress unless specified otherwise.

## Source tree
| Directory	| Description									|
|-----------|-----------------------------------------------|
| (root)	| General/Universal tasks of the kernel			|
| tables	| IDT, GDT 										|
| serial	| Serial I/O									|
| power		| Halt (reboot and poweroff in future)			|
| memory	| Page frame allocator, paging, memory mapping	|
| libk		| Some libc implementations						|
| hw_interrupts	| PIC driver								|
| graphics	| Text rendering								|
| efi		| POSIX-UEFI structs							|

## Resources
This project was made according to Poncho's osdev series and [wiki.osdev](https://wiki.osdev.org/).
