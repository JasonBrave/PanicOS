OBJS = \
	arch/x86/lapic.o\
	arch/x86/msi.o\
	common/sleeplock.o\
	common/spinlock.o\
	common/string.o\
	core/console.o\
	core/kalloc.o\
	core/main.o\
	core/mp.o\
	core/proc.o\
	core/swtch.o\
	core/trap.o\
	core/trapasm.o\
	core/vm.o\
	driver/ahci/ahci.o\
	driver/ata/adapter.o\
	driver/ata/ata.o\
	driver/ata/pata.o\
	driver/ata/sata.o\
	driver/bochs-display/bochs-display.o\
	driver/pci/driver.o\
	driver/pci/intx.o\
	driver/pci/msi.o\
	driver/pci/msix.o\
	driver/pci/pci-kcall.o\
	driver/pci/pci.o\
	driver/ps2/keyboard.o\
	driver/ps2/mouse.o\
	driver/uhci/uhci.o\
	driver/usb/hub.o\
	driver/usb/request.o\
	driver/usb/transfer.o\
	driver/usb/usb.o\
	driver/virtio/virtio-blk.o\
	driver/virtio/virtio.o\
	driver/x86/intel-pcie-mmcfg.o\
	driver/x86/ioapic.o\
	driver/x86/pci-legacy.o\
	driver/x86/pic.o\
	driver/x86/platform.o\
	driver/x86/rtc.o\
	driver/x86/uart.o\
	filesystem/fs-driver.o\
	filesystem/fat32/cluster.o\
	filesystem/fat32/dir.o\
	filesystem/fat32/fat.o\
	filesystem/fat32/fat32.o\
	filesystem/fat32/mount.o\
	filesystem/vfs/dir.o\
	filesystem/vfs/filedesc.o\
	filesystem/vfs/path.o\
	filesystem/vfs/vfs.o\
	hal/block.o\
	hal/display.o\
	hal/gpt.o\
	hal/hid.o\
	hal/mbr.o\
	hal/module.o\
	hal/power.o\
	proc/exec/dynamic.o\
	proc/exec/elf.o\
	proc/exec/exec.o\
	proc/syscall/syscall.o\
	proc/syscall/sysfile.o\
	proc/syscall/sysproc.o\
	proc/kcall.o\
	proc/pty.o\
	vectors.o\

CFLAGS += -I. -fno-builtin
ASFLAGS += -I.

kernel : $(OBJS) core/entry.o entryother initcode kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel core/entry.o $(OBJS) $(shell $(CC) -print-libgcc-file-name) -b binary initcode entryother

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
	rm -f $(OBJS) kernel entryother bootblockother.o initcode.out initcode core/entry.o entryother.o initcode.o
