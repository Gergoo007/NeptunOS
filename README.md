# NeptunOS
is my time sink. Code quality may vary, and it has a bit of undefined behaviour but it's mostly stable. It began as a way to learn about low-level programming, now it's more about learning C++

## Features
- Robust VMM: only allocates physical memory when it is about to be used, and has optional debug checks, using which you can detect memory corruption, and where the corruption occured.
- Modules: dynamic loading of modules, which allows you to implement some generic APIs (ie. init, read/write on mass storage devices, etc.)
- Fast framebuffer: A backbuffer is used here, aligned to a 64 byte boundary, meaning AVX can be used for batch ops like scrolling, clearing the screen. Scrolling is pretty much seamless on real hardware and accelerated VMs.
- Basic scheduler: supports kernel threads as well as userspace programs with their own address spaces.

## Roadmap
### High priority
- Userspace standard library, basic programs
- User input

### Low priority
- ACPI
- XHCI, USB HID
- Porting Rust
