# PanicOS
A simple operating system for 32-bits x86.

## Features
### Kernel
* 32-bits x86 support
* Legacy BIOS and UEFI booting with GRUB2
* SMP
* PAE paging (see [pae](https://github.com/JasonBrave/PanicOS/tree/pae) branch)
* ELF file loading
* Loadable kernel modules
* Parallel ATA IDE Controller and Disk driver
* QEMU Bochs display driver
* PCI Bus support
* PCI Express ECAM support
* PCI MSI and MSI-X interrupt support
* PS/2 keyboard and mose driver
* USB Bus support
* USB Hub driver
* UHCI USB controller driver
* VirtIO block device and GPU device driver
* PC platform RTC and UART driver
* Virtual filesystem
* FAT32 filesystem support
* Master Boot Record partition table support
* Hardware abstraction layer abstracts block device, display device and HID device
* Reboot and shutdown
### User Space
* Dynamic linking
* C++ global constructor and destructor
* [C Standard Library](https://github.com/JasonBrave/PanicOS/tree/master/library/libc)
* [C++ Standard Library including STL](https://github.com/JasonBrave/PanicOS/tree/master/library/libcpp)
* [C++ object-oriented GUI widget toolkit](https://github.com/JasonBrave/PanicOS/tree/master/library/libgui)
* POSIX emulation library
* Window manager
* Graphical desktop envoironment
* Various user space programs

## Build Instructions

A build of gcc and binutils with `i686-elf` target triplet is required. `dosfstools` and `mtools` are also required for creating disk image. QEMU with target i686 or x86-64 is required for emulation

Run `make` to create disk image and kernel image, run `make qemu` to start qemu emulation.

## Directory Structure
* `/boot` Disk image boot sector
* `/ext` Utility for building out-of-tree kernel modules
* `/kernel` Operating System Kernel
* `/kernel/arch/x86` x86 architecture specific code
* `/kernel/common` kernel common used code
* `/kernel/core` Kernel core
* `/kernel/drivers` Kernel built-in device drivers
* `/kernel/filesystem` Kernel build-in filesystem drivers
* `/kernel/hal` Kernel hardware abstraction layer
* `/kernel/proc` Kernel process management
* `/library` User space libraries
* `/library/crt` C Runtime (crt0 and crt1)
* `/library/ld` Dynamic linker
* `/library/libc` C Standard Library
* `/library/libcpp` C++ Standard Library
* `/library/libcpu` CPU abstraction library
* `/library/libgui` C++ GUI widget toolkit library
* `/library/libposix` POSIX emulation library
* `/library/libsys` System call library
* `/library/libwm` Window manager library
* `/module/edu` QEMU EDU device driver
* `/module/hello` Hello World kernel module example
* `/module/modlib` Kernel module support library
* `/module/virtgpu` VirtIO GPU driver
* `/programs/about` Graphical About program
* `/programs/cat` A program to display context of a text file
* `/programs/date` A command line utility to display date and time
* `/programs/desktop` Graphical desktop environment
* `/programs/devmgr` Device manager
* `/programs/dir` A program to list files in a directory
* `/programs/init` Init program
* `/programs/kmod` A tool to load kernel modules
* `/programs/lscpu` An utility to display CPU infomation
* `/programs/lspci` An utility to list PCI (Express) devices
* `/programs/mkdir` A program for creating directories
* `/programs/reboot` A program to reboot computer
* `/programs/rm` A program to delete files
* `/programs/sh` Shell
* `/programs/shutdown` A program to shut down computer
* `/programs/termemu` Graphical terminal emulator
* `/programs/wm` Graphical window manager
* `/share` Non-code data
* `/tools` Tools used in the compiling process
