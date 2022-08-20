PREFIX = i686-elf-
export CC= $(PREFIX)gcc
export CFLAGS= -fno-strict-aliasing -O2 -Wall -Wextra -std=c17 -fno-omit-frame-pointer \
		-fno-stack-protector -gdwarf-5 -Werror-implicit-function-declaration
export CXX= $(PREFIX)g++
export CXXFLAGS= -fno-strict-aliasing -O2 -Wall -Wextra -std=c++20 -fno-omit-frame-pointer -fno-stack-protector \
				-gdwarf-5 -fno-sized-deallocation -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Werror-implicit-function-declaration
export AS= $(PREFIX)as
export ASFLAGS = -gdwarf-5 -Wa,-divide
export LD= $(PREFIX)ld
export OBJCOPY= $(PREFIX)objcopy
export AR = $(PREFIX)ar

all: panicos.img

qemu: panicos.img
	qemu-system-i386 -serial mon:stdio -kernel kernel/kernel -drive file=panicos.img,format=raw,if=virtio \
	-smp 2 -m 128 -net none -rtc base=localtime

qemu-gdb: panicos.img
	qemu-system-i386 -serial mon:stdio -kernel kernel/kernel -drive file=panicos.img,format=raw,if=virtio \
	-smp 2 -m 128 -s -S -net none -rtc base=localtime

qemu-kvm: panicos.img
	qemu-system-i386 -serial mon:stdio -kernel kernel/kernel -drive file=panicos.img,format=raw,if=virtio \
	-smp 2 -m 128 -accel kvm -cpu host -net none -rtc base=localtime

panicos.img: boot/mbr.bin kernel/kernel rootfs program share module
	dd if=/dev/zero of=fs.img bs=1M count=63
	/sbin/mkfs.vfat -F32 -s1 -nPanicOS fs.img
	mcopy -i fs.img -s rootfs/* ::
	dd if=/dev/zero of=panicos.img bs=1M count=64
	dd if=boot/mbr.bin of=panicos.img conv=notrunc
	dd if=fs.img of=panicos.img bs=1M conv=notrunc seek=1
	rm -f fs.img

boot/mbr.bin:
	$(MAKE) -C boot mbr.bin

kernel/kernel: rootfs
	$(MAKE) -C kernel kernel
	cp kernel/kernel rootfs/boot

.PHONY: program
program: library rootfs
	$(MAKE) -C program

.PHONY: library
library: rootfs
	$(MAKE) -C library

.PHONY: share
share: rootfs
	$(MAKE) -C share install

.PHONY: module
module: rootfs
	$(MAKE) -C module

.PHONY: rootfs
rootfs:
	mkdir -p rootfs/bin rootfs/lib rootfs/devel/include rootfs/devel/lib \
	rootfs/share rootfs/boot/module

.PHONY: dist
dist: kernel/kernel rootfs program share module
	tar -czf panicos.tar.gz rootfs/*

.PHONY: clean
clean:
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean
	$(MAKE) -C library clean
	$(MAKE) -C program clean
	$(MAKE) -C module clean
	rm -rf panicos.img rootfs panicos.tar.gz
