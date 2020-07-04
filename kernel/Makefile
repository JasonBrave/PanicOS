OBJS = \
	core/console.o\
	proc/exec/exec.o\
	core/ioapic.o\
	core/kalloc.o\
	core/kbd.o\
	core/lapic.o\
	core/main.o\
	core/mp.o\
	core/picirq.o\
	core/proc.o\
	common/sleeplock.o\
	common/spinlock.o\
	common/string.o\
	core/swtch.o\
	proc/syscall/syscall.o\
	proc/syscall/sysfile.o\
	proc/syscall/sysproc.o\
	core/trapasm.o\
	core/trap.o\
	core/uart.o\
	vectors.o\
	core/vm.o\
	filesystem/initramfs/initramfs.o\
	filesystem/vfs/dir.o\
	filesystem/vfs/vfs.o\
	filesystem/vfs/filedesc.o\
	proc/exec/elf.o\
	driver/pci/pci.o\
	driver/pci/intx.o\
	driver/edu/edu.o\
	driver/virtio-blk/virtio-blk.o\
	hal/block.o\
	filesystem/fat32/mount.o\

CFLAGS += -I.
ASFLAGS += -I.

kernel : $(OBJS) core/entry.o entryother initcode kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel core/entry.o $(OBJS) -b binary initcode entryother

vectors.S: ../tools/vectors.pl
	../tools/vectors.pl > vectors.S

entryother: core/entryother.S
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I. -c core/entryother.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7000 -o bootblockother.o entryother.o
	$(OBJCOPY) -S -O binary -j .text bootblockother.o entryother

initcode: proc/init/initcode.S
	$(CC) $(CFLAGS) -nostdinc -I. -c proc/init/initcode.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o initcode.out initcode.o
	$(OBJCOPY) -S -O binary initcode.out initcode

.PHONY: clean
clean:
	rm -f $(OBJS)  vectors.S kernel entryother bootblockother.o initcode.out initcode core/entry.o entryother.o initcode.o
