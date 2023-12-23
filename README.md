# Operating System

## TODO
- [X] Bootable on UEFI
- [X] Runs on both 256 MiB-s of RAM and 256 GiB
- [X] Paging, VMM, PMM, request_page, map_page, free_page
- [ ] Early printk (runs before paging is set up) and full printk (faster, no limitations)
- [ ] IDT, GDT
- [ ] Kernel panic screen with backtrace, exception details and registers
- [ ] TSS
- [ ] Proper graphics routines (eg. draw_rect, draw_circle)
- [ ] Detect PCIe devices, use them
- [X] ACPI tables
- [ ] IOAPIC, LAPIC
- [ ] AHCI read and write
- [ ] USB
- [ ] USB keyboard and mouse
- [ ] TTY-like console
- [ ] Jumping between ring 0 and ring 3 (userspace)

## How it boots
- GRUB loads the preloader, whose data section contains the actual kernel
- The preloader maps the addresses specified in the kernel image (the segments)
- kmain() get called
- Multiboot tags are processed
- Console gets initialized
- The PMM and then the VMM gets initialized
- The pages occupied by the framebuffer and the kernel are marked as used
- ACPI tables are interpreted (enumerated)
- kmain() halts the processor
