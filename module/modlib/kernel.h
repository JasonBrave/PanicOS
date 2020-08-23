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

#ifndef _MODLIB_KERNEL_H
#define _MODLIB_KERNEL_H

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

// driver/pci/pci.h
struct PciAddress {
	int bus, device, function;
};

struct PCIDevice {
	struct PciAddress addr;
	uint16_t vendor_id, device_id;
	uint8_t class, subclass, progif;
	uint8_t irq;
	void* private;
	void (*intx_intr_handler)(struct PCIDevice*);
	const struct PCIDriver* driver;
};

struct PCIDeviceID {
	uint16_t vendor_id, device_id;
};

struct PCIDriver {
	const char* name;
	const struct PCIDeviceID* match_table;
	unsigned int class_type;
	void (*init)(struct PCIDevice*);
};

// hal/hal.h
struct BlockDeviceDriver {
	int (*block_read)(void* private, unsigned int begin, int count, void* buf);
	int (*block_write)(void* private, unsigned int begin, int count, const void* buf);
};

struct FramebufferDriver {
	phyaddr_t (*enable)(void* private, int xres, int yres);
	void (*disable)(void* private);
	void (*update)(void* private);
	unsigned int (*read_edid)(void* private, unsigned int bytes);
};

const static struct KernerServiceTable {
	// basic functions
	void (*cprintf)(const char*, ...);
	void (*panic)(const char*);
	void* (*kalloc)(void);
	void (*kfree)(void*);
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
	void (*pci_enable_bus_mastering)(const struct PciAddress*);
	int (*pci_find_capability)(const struct PciAddress*, uint8_t);
	void (*pci_register_intr_handler)(struct PCIDevice*, void (*)(struct PCIDevice*));
	void (*pci_enable_intx_intr)(const struct PciAddress*);
	void (*pci_disable_intx_intr)(const struct PciAddress*);
	int (*pci_msi_alloc_vector)(void (*)(void*), void*);
	void (*pci_msi_free_vector)(int);
	int (*pci_msi_enable)(const struct PciAddress*, int, int);
	void (*pci_msi_disable)(const struct PciAddress*);
	void (*pci_register_driver)(const struct PCIDriver*);
	// hal/hal.h
	void (*hal_block_register_device)(const char*, void*,
									  const struct BlockDeviceDriver*);
	void (*hal_display_register_device)(const char*, void*, struct FramebufferDriver*);
}* kernsrv = (void*)0x80010000;

// common functions
#define cprintf(format, ...) kernsrv->cprintf(format, ##__VA_ARGS__);

static inline void panic(const char* s) {
	return kernsrv->panic(s);
}

static inline void* kalloc(void) {
	return kernsrv->kalloc();
}

static inline void kfree(void* ptr) {
	return kernsrv->kfree(ptr);
}

// common/spinlock.h
static inline void initlock(struct spinlock* lock, const char* name) {
	return kernsrv->initlock(lock, name);
}

static inline void acquire(struct spinlock* lock) {
	return kernsrv->acquire(lock);
}

static inline void release(struct spinlock* lock) {
	return kernsrv->release(lock);
}

// driver/pci/pci.h
static inline uint8_t pci_read_config_reg8(const struct PciAddress* addr, int reg) {
	return kernsrv->pci_read_config_reg8(addr, reg);
}

static inline uint16_t pci_read_config_reg16(const struct PciAddress* addr, int reg) {
	return kernsrv->pci_read_config_reg16(addr, reg);
}

static inline uint32_t pci_read_config_reg32(const struct PciAddress* addr, int reg) {
	return kernsrv->pci_read_config_reg32(addr, reg);
}

static inline void pci_write_config_reg8(const struct PciAddress* addr, int reg,
										 uint8_t data) {
	return kernsrv->pci_write_config_reg8(addr, reg, data);
}

static inline void pci_write_config_reg16(const struct PciAddress* addr, int reg,
										  uint16_t data) {
	return kernsrv->pci_write_config_reg16(addr, reg, data);
}

static inline void pci_write_config_reg32(const struct PciAddress* addr, int reg,
										  uint32_t data) {
	return kernsrv->pci_write_config_reg32(addr, reg, data);
}

static inline phyaddr_t pci_read_bar(const struct PciAddress* addr, int bar) {
	return kernsrv->pci_read_bar(addr, bar);
}

static inline void pci_enable_bus_mastering(const struct PciAddress* addr) {
	return kernsrv->pci_enable_bus_mastering(addr);
}

static inline int pci_find_capability(const struct PciAddress* addr, uint8_t cap_id) {
	return kernsrv->pci_find_capability(addr, cap_id);
}

static inline void pci_register_intr_handler(struct PCIDevice* dev,
											 void (*handler)(struct PCIDevice*)) {
	return kernsrv->pci_register_intr_handler(dev, handler);
}

static inline void pci_enable_intx_intr(const struct PciAddress* addr) {
	return kernsrv->pci_enable_intx_intr(addr);
}
static inline void pci_disable_intx_intr(const struct PciAddress* addr) {
	return kernsrv->pci_disable_intx_intr(addr);
}

static inline int pci_msi_alloc_vector(void (*handler)(void*), void* private) {
	return kernsrv->pci_msi_alloc_vector(handler, private);
}

static inline void pci_msi_free_vector(int vector) {
	return kernsrv->pci_msi_free_vector(vector);
}

static inline int pci_msi_enable(const struct PciAddress* addr, int vector,
								 int lapicid) {
	return kernsrv->pci_msi_enable(addr, vector, lapicid);
}

static inline void pci_msi_disable(const struct PciAddress* addr) {
	return kernsrv->pci_msi_disable(addr);
}

static inline void pci_register_driver(const struct PCIDriver* driver) {
	return kernsrv->pci_register_driver(driver);
}

static inline void hal_block_register_device(const char* name, void* private,
											 const struct BlockDeviceDriver* driver) {
	return kernsrv->hal_block_register_device(name, private, driver);
}

static inline void hal_display_register_device(const char* name, void* private,
											   struct FramebufferDriver* driver) {
	return kernsrv->hal_display_register_device(name, private, driver);
}

#define KERNBASE 0x80000000 // First kernel virtual address

#define V2P(a) (((unsigned int)(a)) - KERNBASE)
#define P2V(a) ((void*)(((char*)(a)) + KERNBASE))

#endif
