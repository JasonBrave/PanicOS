ifndef PANICOS_PATH
$(error set variable PANICOS_PATH to PanicOS source tree path)
endif

PREFIX = i686-elf-
CC= $(PREFIX)gcc
CFLAGS= -fno-strict-aliasing -O2 -Wall -Wextra -std=c17 -fno-omit-frame-pointer \
		-fno-stack-protector -gdwarf-5
LD= $(PREFIX)ld
CFLAGS += -I$(PANICOS_PATH)module/modlib -fPIE -fno-builtin

$(MOD).mod : $(OBJS)
	$(LD) -pie --no-dynamic-linker -e module_init $(OBJS) -o $(MOD).mod

.PHONY: install
install: $(MOD).mod
	cp $(MOD).mod ..

.PHONY: clean
clean:
	rm -f $(MOD).mod $(OBJS) panicos.img

qemu: panicos.img
	qemu-system-i386 -serial mon:stdio -kernel $(PANICOS_PATH)/kernel/kernel -drive file=panicos.img,format=raw,if=virtio \
	-smp 2 -m 128 -net none -rtc base=localtime $(QEMU_ARGS)

qemu-gdb: panicos.img
	qemu-system-i386 -serial mon:stdio -kernel $(PANICOS_PATH)/kernel/kernel -drive file=panicos.img,format=raw,if=virtio \
	-smp 2 -m 128 -s -S -net none -rtc base=localtime $(QEMU_ARGS)

qemu-kvm: panicos.img
	qemu-system-i386 -serial mon:stdio -kernel $(PANICOS_PATH)/kernel/kernel -drive file=panicos.img,format=raw,if=virtio \
	-smp 2 -m 128 -accel kvm -cpu host -net none -rtc base=localtime $(QEMU_ARGS)

panicos.img: $(MOD).mod
	dd if=/dev/zero of=fs.img bs=1M count=63
	/sbin/mkfs.vfat -F32 -s8 -nPanicOS fs.img
	mcopy -i fs.img -bs $(PANICOS_PATH)/rootfs/* ::
	mcopy -i fs.img -bs $(MOD).mod ::/boot/module
	dd if=/dev/zero of=panicos.img bs=1M count=64
	dd if=$(PANICOS_PATH)/boot/mbr.bin of=panicos.img conv=notrunc
	dd if=fs.img of=panicos.img bs=1M conv=notrunc seek=1
	rm -f fs.img
