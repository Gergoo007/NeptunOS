# NeptunOS
is my attempt at a hobby operating system. This is my second bigger project (the first being the 6502 emulator), and my first OS.

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

## Credits
- [osdev.org](https://wiki.osdev.org/)
- [Poncho](https://www.youtube.com/@poncho2364)
- [bzt](https://gitlab.com/bztsrc)
