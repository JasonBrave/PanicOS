/*
 * Kernel module library
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

#ifndef _MODLIB_KERNSRV_H
#define _MODLIB_KERNSRV_H

#include <kernel-types.h>

// common/errorcode.h
#define ERROR_INVAILD -1
#define ERROR_NOT_EXIST -2
#define ERROR_EXIST -3
#define ERROR_NOT_FILE -4
#define ERROR_NOT_DIRECTORY -5
#define ERROR_READ_FAIL -6
#define ERROR_OUT_OF_SPACE -7
#define ERROR_WRITE_FAIL -8
#define ERROR_NO_PERM -9

// common/spinlock.h
struct spinlock {
	unsigned int locked; // Is the lock held?
	const char* name; // Name of lock.
	struct cpu* cpu; // The cpu holding the lock.
	unsigned int pcs[10];
};

struct PciAddress;
struct PCIDevice;
struct PCIDriver;
struct VirtioDriver;
struct VirtioDevice;
struct VirtioQueue;
struct BlockDeviceDriver;
struct FramebufferDriver;
struct USBDevice;
struct USBBus;
struct USBDriver;
struct USBHostControllerDriver;
struct MSIMessage;

const static struct KernerServiceTable {
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

#define KERNBASE 0x80000000 // First kernel virtual address
#define DEVSPACE 0xB0000000 // Low MMIO

#define V2P(a) (((unsigned int)(a)) - KERNBASE)
#define P2V(a) ((void*)(((char*)(a)) + KERNBASE))

#endif
