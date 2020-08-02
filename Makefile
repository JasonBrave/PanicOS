PREFIX = i686-elf-
export CC= $(PREFIX)gcc
export CFLAGS= -fno-strict-aliasing -O2 -Wall -std=c17 -fno-omit-frame-pointer \
		-fno-stack-protector -gdwarf-4 -Werror
export CXX= $(PREFIX)g++
export CXXFLAGS= -fno-strict-aliasing -O2 -Wall -Werror -std=c++20 -fno-omit-frame-pointer -fno-stack-protector \
				-gdwarf-4 -fno-sized-deallocation -fno-exceptions -fno-rtti -fno-use-cxa-atexit
export AS= $(PREFIX)as
export ASFLAGS = -gdwarf-4 -Wa,-divide
export LD= $(PREFIX)ld
export OBJCOPY= $(PREFIX)objcopy
export AR = $(PREFIX)ar

all: panicos.img

qemu: panicos.img
	qemu-system-i386 -serial mon:stdio -kernel kernel/kernel -drive file=panicos.img,format=raw,if=virtio \
	-smp 2 -m 256 -net none

qemu-gdb: panicos.img
	qemu-system-i386 -serial mon:stdio -kernel kernel/kernel -drive file=panicos.img,format=raw,if=virtio \
	-smp 2 -m 256 -s -S -net none

qemu-kvm: panicos.img
	qemu-system-i386 -serial mon:stdio -kernel kernel/kernel -drive file=panicos.img,format=raw,if=virtio \
	-smp 2 -m 256 -accel kvm -cpu host -net none

panicos.img: boot/mbr.bin kernel/kernel rootfs program
	dd if=/dev/zero of=fs.img bs=1M count=63
	mkfs.vfat -F32 -s8 -nPanicOS fs.img
	mcopy -i fs.img -bs rootfs/* ::
	dd if=/dev/zero of=panicos.img bs=1M count=64
	dd if=boot/mbr.bin of=panicos.img conv=notrunc
	dd if=fs.img of=panicos.img bs=1M conv=notrunc seek=1
	rm -f fs.img

boot/mbr.bin:
	$(MAKE) -C boot mbr.bin

kernel/kernel:
	$(MAKE) -C kernel kernel

.PHONY: program
program: library rootfs
	$(MAKE) -C program

.PHONY: library
library: rootfs
	$(MAKE) -C library

.PHONY: rootfs
rootfs:
	mkdir -p rootfs/bin rootfs/lib rootfs/devel/include rootfs/devel/lib

.PHONY: clean
clean:
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean
	$(MAKE) -C library clean
	$(MAKE) -C program clean
	rm -rf panicos.img rootfs
