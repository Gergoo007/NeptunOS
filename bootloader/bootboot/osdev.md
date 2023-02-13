OSDev Lies
==========

Sadly that place isn't the place it used to be. Here are a few examples as a proof, lies spread most notably by
nexos and Octocontrabass:

["No, it does not."](https://forum.osdev.org/viewtopic.php?f=2&t=33362&p=337968&hilit=bootboot#p337968)

Yes, **it does**. BOOTBOOT does provide the same environment and a consistent, architecture-independent memory map on ALL supported platforms, see for yourself.


["BOOTBOOT is very finicky from what I've seen."](https://forum.osdev.org/viewtopic.php?f=1&t=56430&p=343068&hilit=bootboot#p343068)

A lie, and just an opinion based on bad education. Limine has exactly the same bad design decisions like GRUB, just far less tested, and it does not have a bootable image creator either.


["No, that's normal for BOOTBOOT."](https://forum.osdev.org/viewtopic.php?f=1&t=56430&p=343010&hilit=bootboot#p343010)

No, it's absolutely not.


["And it looks like BOOTBOOT doesn't correctly align the stack according to the System V ABI..."](https://forum.osdev.org/viewtopic.php?f=1&t=56430&p=343000&hilit=bootboot#p343000)

Bullshit, a clear cut lie, proved wrong by multiple working kernels using BOOTBOOT. (Since when address 0 isn't aligned with *anything*?)


["And is BOOTBOOT really a sane bootloader if it places arbitrary limits on the kernel size?"](https://forum.osdev.org/viewtopic.php?f=1&t=56430&p=342990&hilit=bootboot#p342990)

Show me one bootloader which does not limit the size. The memory has limits, no bootloader can load infinite size kernels. And does that carefully choosen 16Mb really concern you? My current Linux kernel is 10Mb, and man, that's not a hobby microkernel, but a bloated monolithic beast.


["BOOTBOOT is much more limited than whichever bootloader you're using, so your linker script won't work with BOOTBOOT."](https://forum.osdev.org/viewtopic.php?f=1&t=56430&p=342916&hilit=bootboot#p342916)

Another lie. BOOTBOOT requires your kernel to be linked at higher half to save you from the gdt trickery and trampoline mess, but that's not a limitation, quite the contrary.


["Now, it would be best practice to do what Limine does and create entries specifically for this kind of memory"](https://forum.osdev.org/viewtopic.php?f=1&t=51891&p=333429&hilit=bootboot#p333429)

Wrong. The reason why BOOTBOOT doesn't have such entries, because it does not pollute your memory, all bootloader memory is already freed by the time your kernel runs. If you really want to know the physical address of your own kernel, look it up in the initrd or walk the page tables.


Some more lies, unrelated to this project:

["I can safely say that Microsoft wasn't involved in the design until after the original EFI ABI and API were developed"](https://forum.osdev.org/viewtopic.php?p=331940#p331940)

Never mind it's acknowledged in the spec... and EFI just accidentally using MS' patented file system, MS' execute format, MS' ABI, MS' API, manufacturers ship MS' keys exclusively as it happens ...etc. But MS wasn't involved in EFI, yeah, sure...


["SDA specification does require using only exFAT for SDXC cards"](https://forum.osdev.org/viewtopic.php?f=15&t=42387&p=320913&hilit=sdhc+exfat#p320913)

No comment. Tell the Raspberry Pi guys they are just imagining booting Raspbian from ext4...

...etc. Long story short, avoid OSDev forum, it's now overtaken by paid trolls and has a very toxic athmosphere, where only klange's OS is considered a valid hobby OS.
