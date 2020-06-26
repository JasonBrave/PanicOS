PREFIX = i686-elf-
export CC= $(PREFIX)gcc
export CFLAGS= -fno-strict-aliasing -O2 -Wall -std=c17 -fno-omit-frame-pointer -fno-stack-protector -gdwarf-4 -fno-builtin -Werror
export AS= $(PREFIX)as
export ASFLAGS = -gdwarf-4 -Wa,-divide
export LD= $(PREFIX)ld
export OBJCOPY= $(PREFIX)objcopy
export AR = $(PREFIX)ar

all: holeos.img fs.img

qemu: holeos.img fs.img
	qemu-system-i386 -serial mon:stdio -drive file=fs.img,index=1,media=disk,format=raw -drive file=holeos.img,index=0,media=disk,format=raw -smp 2 -m 512 -net none

qemu-gdb: holeos.img fs.img
	qemu-system-i386 -serial mon:stdio -drive file=fs.img,index=1,media=disk,format=raw -drive file=holeos.img,index=0,media=disk,format=raw -smp 2 -m 512 -s -S -net none

qemu-kvm: holeos.img fs.img
	qemu-system-i386 -serial mon:stdio -drive file=fs.img,index=1,media=disk,format=raw -drive file=holeos.img,index=0,media=disk,format=raw -smp 2 -m 512 -accel kvm -cpu host -net none

holeos.img: boot/bootblock kernel/kernel rootfs.cpio
	dd if=/dev/zero of=holeos.img count=10000
	dd if=boot/bootblock of=holeos.img conv=notrunc
	dd if=kernel/kernel of=holeos.img seek=1 conv=notrunc
	dd if=rootfs.cpio of=holeos.img seek=8192 conv=notrunc

fs.img: tools/xv6fs program
	$(MAKE) -C rootfs fs.img

rootfs.cpio: program
	$(MAKE) -C rootfs rootfs.cpio

boot/bootblock:
	$(MAKE) -C boot bootblock

kernel/kernel:
	$(MAKE) -C kernel kernel

tools/xv6fs:
	$(MAKE) -C tools xv6fs

.PHONY: program
program: lib/lib.a
	$(MAKE) -C program

lib/lib.a:
	$(MAKE) -C lib lib.a

.PHONY: clean
clean:
	$(MAKE) -C tools clean
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean
	$(MAKE) -C lib clean
	$(MAKE) -C program clean
	$(MAKE) -C rootfs clean
	rm -f holeos.img fs.img rootfs.cpio
