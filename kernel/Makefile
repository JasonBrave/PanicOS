OBJS_COMMON = \
	common/string.o\
	core/console.o\
	core/kalloc.o\
	core/main.o\
	driver/pci/driver.o\
	driver/pci/intx.o\
	driver/pci/msi.o\
	driver/pci/msix.o\
	driver/pci/pci-kcall.o\
	driver/pci/pci.o\
	driver/usb/hub.o\
	driver/usb/request.o\
	driver/usb/transfer.o\
	driver/usb/usb.o\
	driver/virtio/virtio-blk.o\
	driver/virtio/virtio.o\
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
	hal/power.o\
	proc/kcall.o\

OBJS_X86 = \
	arch/x86/lapic.o\
	arch/x86/msi.o\
	common/sleeplock.o\
	arch/x86/spinlock.o\
	arch/x86/mp.o\
	core/proc.o\
	arch/x86/swtch.o\
	arch/x86/trap.o\
	arch/x86/trapasm.o\
	arch/x86/vm.o\
	driver/ahci/ahci.o\
	driver/ata/adapter.o\
	driver/ata/ata.o\
	driver/ata/pata.o\
	driver/ata/sata.o\
	driver/bochs-display/bochs-display.o\
	driver/ps2/keyboard.o\
	driver/ps2/mouse.o\
	driver/uhci/uhci.o\
	driver/x86/intel-pcie-mmcfg.o\
	driver/x86/ioapic.o\
	driver/x86/pci-legacy.o\
	driver/x86/pic.o\
	driver/x86/platform.o\
	driver/x86/rtc.o\
	driver/x86/uart.o\
	hal/module.o\
	proc/exec/dynamic.o\
	proc/exec/elf.o\
	proc/exec/exec.o\
	proc/syscall/syscall.o\
	proc/syscall/sysfile.o\
	proc/syscall/sysproc.o\
	proc/pty.o\
	vectors.o\

OBJS_RISCV = \
	arch/riscv/imsic.o\
	arch/riscv/spinlock.o\
	arch/riscv/vm.o\
	driver/riscv/ecam.o\
	driver/riscv/platform.o

ifeq ($(ARCH),riscv)
OBJS = $(OBJS_COMMON) $(OBJS_RISCV)
endif

ifeq ($(ARCH),x86)
OBJS = $(OBJS_COMMON) $(OBJS_X86)
endif

ifeq ($(ARCH),x86)
X86DEP = entryother initcode
X86LDEXTRA = -b binary $(X86DEP)
endif

CFLAGS += -I. -fno-builtin
ASFLAGS += -I.

kernel : $(OBJS) arch/$(ARCH)/entry.o arch/$(ARCH)/kernel.ld $(X86DEP)
	$(LD) $(LDFLAGS) -T arch/$(ARCH)/kernel.ld -o kernel arch/$(ARCH)/entry.o $(OBJS) $(shell $(CC) -print-libgcc-file-name) $(X86LDEXTRA)

entryother: arch/x86/entryother.S
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I. -c arch/x86/entryother.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7000 -o bootblockother.o entryother.o
	$(OBJCOPY) -S -O binary -j .text bootblockother.o entryother

initcode: proc/init/initcode.S
	$(CC) $(CFLAGS) -nostdinc -I. -c proc/init/initcode.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o initcode.out initcode.o
	$(OBJCOPY) -S -O binary initcode.out initcode

.PHONY: clean
clean:
	rm -f $(OBJS_COMMON) $(OBJS_X86) $(OBJS_RISCV) kernel entryother bootblockother.o initcode.out initcode entryother.o initcode.o arch/x86/entry.o arch/riscv/entry.o
