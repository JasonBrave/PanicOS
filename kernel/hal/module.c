/*
 * Kernel module loader
 *
 * This file is part of PanicOS.
 *
 * PanicOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PanicOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PanicOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <arch/x86/msi.h>
#include <common/spinlock.h>
#include <defs.h>
#include <driver/pci/pci.h>
#include <driver/usb/usb.h>
#include <driver/virtio/virtio.h>
#include <filesystem/vfs/vfs.h>
#include <hal/hal.h>
#include <proc/exec/elf.h>

#define MAX_MODULES 64

struct ModuleInfo {
	char name[64];
	unsigned int base;
} module_info[MAX_MODULES];

static int module_elf_load(pdpte_t* pgdir, unsigned int base, const char* name, unsigned int* entry) {
	struct FileDesc fd;
	if (vfs_fd_open(&fd, name, O_READ) < 0) {
		return -1;
	}

	struct elfhdr elf;
	if (vfs_fd_read(&fd, (char*)&elf, sizeof(elf)) != sizeof(elf)) {
		vfs_fd_close(&fd);
		return -1;
	}
	if (elf.magic != ELF_MAGIC) {
		vfs_fd_close(&fd);
		return -1;
	}
	*entry = elf.entry;

	struct proghdr ph;
	unsigned int sz = 0;
	for (unsigned int off = elf.phoff; off < elf.phoff + elf.phnum * sizeof(ph);
		 off += sizeof(ph)) {
		if (vfs_fd_seek(&fd, off, SEEK_SET) < 0) {
			vfs_fd_close(&fd);
			return -1;
		}
		if (vfs_fd_read(&fd, (char*)&ph, sizeof(ph)) != sizeof(ph)) {
			vfs_fd_close(&fd);
			return -1;
		}
		if (ph.type != ELF_PROG_LOAD) {
			continue;
		}
		if (allocuvm(pgdir, base + ph.vaddr - ph.vaddr % PGSIZE, base + ph.vaddr + ph.memsz,
					 (ph.flags & ELF_PROG_FLAG_WRITE) ? PTE_W : 0) == 0) {
			vfs_fd_close(&fd);
			return -1;
		}
		if (ph.vaddr + ph.memsz > sz) {
			sz = ph.vaddr + ph.memsz;
		}
		if (loaduvm(pgdir, (char*)base + ph.vaddr - ph.vaddr % PGSIZE, &fd,
					ph.off - ph.vaddr % PGSIZE, ph.vaddr % PGSIZE + ph.filesz) < 0) {
			vfs_fd_close(&fd);
			return -1;
		}
	}

	vfs_fd_close(&fd);

	return sz;
}

static unsigned int module_base = PROC_MODULE_BOTTOM;
extern pdpte_t* kpgdir;

void module_info_add(const char* name, unsigned int base) {
	for (int i = 0; i < MAX_MODULES; i++) {
		if (!module_info[i].base) {
			strncpy(module_info[i].name, name, 64);
			module_info[i].base = base;
			break;
		}
	}
}

int module_load(const char* name) {
	unsigned int entry;
	unsigned int load_base = module_base;
	int ret = module_elf_load(kpgdir, load_base, name, &entry);
	if (ret < 0) {
		return ret;
	}
	module_info_add(name, load_base);
	acquire(&ptable.lock);
	for (int i = 0; i < NPROC; i++) {
		if (ptable.proc[i].state != UNUSED && ptable.proc[i].state != EMBRYO) {
			if (copypgdir(ptable.proc[i].pgdir, kpgdir, load_base, load_base + ret) == 0) {
				panic("module load copy failed");
			}
		}
	}
	release(&ptable.lock);
	module_base += PGROUNDUP(ret);
	void (*module_entry_point)(void) = (void (*)(void))(load_base + entry);
	module_entry_point();
	return 0;
}

void module_set_pgdir(pdpte_t* pgdir) {
	if (copypgdir(pgdir, kpgdir, PROC_MODULE_BOTTOM, module_base) == 0) {
		panic("module_set_pgdir failed");
	}
}

void module_print(void) {
	cprintf("Kernel modules:\n");
	for (int i = 0; i < MAX_MODULES; i++) {
		if (module_info[i].base) {
			cprintf("%x %s\n", module_info[i].base, module_info[i].name);
		}
	}
}

static struct KernerServiceTable {
	// basic functions
	void (*cprintf)(const char*, ...);
	void (*panic)(const char*);
	void* (*pgalloc)(unsigned int);
	void (*pgfree)(void*, unsigned int);
	void* (*map_mmio_region)(phyaddr_t, size_t);
	void* (*map_ram_region)(phyaddr_t, size_t);
	void* (*map_rom_region)(phyaddr_t, size_t);
	// arch specific
	int (*msi_alloc_vector)(struct MSIMessage* msg, void (*handler)(void*), void* private);
	void (*msi_free_vector)(const struct MSIMessage* msg);
	// process control
	void (*sleep)(void*, struct spinlock*);
	void (*wakeup)(void*);
	// common/spinlock.h
	void (*initlock)(struct spinlock*, const char*);
	void (*acquire)(struct spinlock*);
	void (*release)(struct spinlock*);
	// driver/pci/pci.h
	uint8_t (*pci_read_config_reg8)(const struct PciAddress*, int);
	uint16_t (*pci_read_config_reg16)(const struct PciAddress*, int);
	uint32_t (*pci_read_config_reg32)(const struct PciAddress*, int);
	void (*pci_write_config_reg8)(const struct PciAddress*, int, uint8_t);
	void (*pci_write_config_reg16)(const struct PciAddress*, int, uint16_t);
	void (*pci_write_config_reg32)(const struct PciAddress*, int, uint32_t);
	phyaddr_t (*pci_read_bar)(const struct PciAddress*, int);
	size_t (*pci_read_bar_size)(const struct PciAddress*, int);
	phyaddr_t (*pci_read_rom_bar)(const struct PciAddress*);
	size_t (*pci_read_rom_bar_size)(const struct PciAddress*);
	void (*pci_enable_bus_mastering)(const struct PciAddress*);
	int (*pci_find_capability)(const struct PciAddress*, uint8_t);
	void (*pci_register_intr_handler)(struct PCIDevice*, void (*)(struct PCIDevice*));
	void (*pci_enable_intx_intr)(const struct PciAddress*);
	void (*pci_disable_intx_intr)(const struct PciAddress*);
	int (*pci_msi_enable)(const struct PciAddress*, const struct MSIMessage*);
	void (*pci_msi_disable)(const struct PciAddress*);
	int (*pci_msix_enable)(struct PCIDevice*);
	unsigned int (*pci_msix_get_num_vectors)(struct PCIDevice*);
	void (*pci_msix_set_message)(struct PCIDevice*, unsigned int, const struct MSIMessage*);
	void (*pci_msix_mask)(struct PCIDevice*, unsigned int);
	void (*pci_msix_unmask)(struct PCIDevice*, unsigned int);
	void (*pci_register_driver)(const struct PCIDriver*);
	// driver/virtio/virtio.h
	void (*virtio_register_driver)(const struct VirtioDriver*);
	void (*virtio_init_queue)(struct VirtioDevice*, struct VirtioQueue*, unsigned int,
							  void (*)(struct VirtioQueue*));
	// driver/usb/usb.h
	void (*usb_register_host_controller)(void*, const char*, unsigned int,
										 const struct USBHostControllerDriver*);
	void (*usb_register_driver)(const struct USBDriver*);
	enum USBTransferStatus (*usb_control_transfer_in)(struct USBBus*, unsigned int, unsigned int,
													  void*, void*, int);
	enum USBTransferStatus (*usb_control_transfer_nodata)(struct USBBus*, unsigned int,
														  unsigned int, void*);
	int (*usb_get_standard_descriptor)(struct USBDevice*, unsigned int, unsigned int, void*,
									   unsigned int);
	int (*usb_get_class_descriptor)(struct USBDevice*, unsigned int, unsigned int, void*,
									unsigned int);
	int (*usb_get_device_descriptor)(struct USBDevice*);
	int (*usb_get_configuration_descriptor)(struct USBDevice*, unsigned int, uint8_t*);
	int (*usb_set_configuration)(struct USBDevice*, uint8_t);
	// hal/hal.h
	void (*hal_block_register_device)(const char*, void*, const struct BlockDeviceDriver*);
	void (*hal_display_register_device)(const char*, void*, const struct FramebufferDriver*);
	void (*hal_mouse_update)(unsigned int);
	void (*hal_keyboard_update)(unsigned int);
}* kernsrv = (void*)0x80010000;

void module_init(void) {
	memset(module_info, 0, sizeof(module_info));
	kernsrv->cprintf = cprintf;
	kernsrv->panic = panic;
	kernsrv->pgalloc = pgalloc;
	kernsrv->pgfree = pgfree;
	kernsrv->map_mmio_region = map_mmio_region;
	kernsrv->map_ram_region = map_ram_region;
	kernsrv->map_rom_region = map_rom_region;
	kernsrv->msi_alloc_vector = msi_alloc_vector;
	kernsrv->msi_free_vector = msi_free_vector;
	kernsrv->sleep = sleep;
	kernsrv->wakeup = wakeup;
	kernsrv->initlock = initlock;
	kernsrv->acquire = acquire;
	kernsrv->release = release;
	kernsrv->pci_read_config_reg8 = pci_read_config_reg8;
	kernsrv->pci_read_config_reg16 = pci_read_config_reg16;
	kernsrv->pci_read_config_reg32 = pci_read_config_reg32;
	kernsrv->pci_write_config_reg8 = pci_write_config_reg8;
	kernsrv->pci_write_config_reg16 = pci_write_config_reg16;
	kernsrv->pci_write_config_reg32 = pci_write_config_reg32;
	kernsrv->pci_read_bar = pci_read_bar;
	kernsrv->pci_read_bar_size = pci_read_bar_size;
	kernsrv->pci_read_rom_bar = pci_read_rom_bar;
	kernsrv->pci_read_rom_bar_size = pci_read_rom_bar_size;
	kernsrv->pci_enable_bus_mastering = pci_enable_bus_mastering;
	kernsrv->pci_find_capability = pci_find_capability;
	kernsrv->pci_register_intr_handler = pci_register_intr_handler;
	kernsrv->pci_enable_intx_intr = pci_enable_intx_intr;
	kernsrv->pci_disable_intx_intr = pci_disable_intx_intr;
	kernsrv->pci_msi_enable = pci_msi_enable;
	kernsrv->pci_msi_disable = pci_msi_disable;
	kernsrv->pci_msix_enable = pci_msix_enable;
	kernsrv->pci_msix_get_num_vectors = pci_msix_get_num_vectors;
	kernsrv->pci_msix_set_message = pci_msix_set_message;
	kernsrv->pci_msix_mask = pci_msix_mask;
	kernsrv->pci_msix_unmask = pci_msix_unmask;
	kernsrv->pci_register_driver = pci_register_driver;
	kernsrv->virtio_register_driver = virtio_register_driver;
	kernsrv->virtio_init_queue = virtio_init_queue;
	kernsrv->usb_register_host_controller = usb_register_host_controller;
	kernsrv->usb_register_driver = usb_register_driver;
	kernsrv->usb_control_transfer_in = usb_control_transfer_in;
	kernsrv->usb_control_transfer_nodata = usb_control_transfer_nodata;
	kernsrv->usb_get_standard_descriptor = usb_get_standard_descriptor;
	kernsrv->usb_get_class_descriptor = usb_get_class_descriptor;
	kernsrv->usb_get_device_descriptor = usb_get_device_descriptor;
	kernsrv->usb_get_configuration_descriptor = usb_get_configuration_descriptor;
	kernsrv->usb_set_configuration = usb_set_configuration;
	kernsrv->hal_block_register_device = hal_block_register_device;
	kernsrv->hal_display_register_device = hal_display_register_device;
	kernsrv->hal_mouse_update = hal_mouse_update;
	kernsrv->hal_keyboard_update = hal_keyboard_update;
}
