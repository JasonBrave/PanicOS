/*
 * Kernel module PCI helper functions
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

#ifndef _MODLIB_PCI_H
#define _MODLIB_PCI_H

#include <kernsrv.h>

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
	// MSI-X specific
	volatile uint32_t* msix_table;
	volatile uint32_t* msix_pba;
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

struct MSIMessage;

static inline uint8_t pci_read_config_reg8(const struct PciAddress* addr, int reg) {
	return kernsrv->pci_read_config_reg8(addr, reg);
}

static inline uint16_t pci_read_config_reg16(const struct PciAddress* addr, int reg) {
	return kernsrv->pci_read_config_reg16(addr, reg);
}

static inline uint32_t pci_read_config_reg32(const struct PciAddress* addr, int reg) {
	return kernsrv->pci_read_config_reg32(addr, reg);
}

static inline void pci_write_config_reg8(const struct PciAddress* addr, int reg, uint8_t data) {
	return kernsrv->pci_write_config_reg8(addr, reg, data);
}

static inline void pci_write_config_reg16(const struct PciAddress* addr, int reg, uint16_t data) {
	return kernsrv->pci_write_config_reg16(addr, reg, data);
}

static inline void pci_write_config_reg32(const struct PciAddress* addr, int reg, uint32_t data) {
	return kernsrv->pci_write_config_reg32(addr, reg, data);
}

static inline phyaddr_t pci_read_bar(const struct PciAddress* addr, int bar) {
	return kernsrv->pci_read_bar(addr, bar);
}

static inline size_t pci_read_bar_size(const struct PciAddress* addr, int bar) {
	return kernsrv->pci_read_bar_size(addr, bar);
}

static inline phyaddr_t pci_read_rom_bar(const struct PciAddress* addr) {
	return kernsrv->pci_read_rom_bar(addr);
}

static inline size_t pci_read_rom_bar_size(const struct PciAddress* addr) {
	return kernsrv->pci_read_rom_bar_size(addr);
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

static inline int pci_msi_enable(const struct PciAddress* addr, const struct MSIMessage* msg) {
	return kernsrv->pci_msi_enable(addr, msg);
}

static inline void pci_msi_disable(const struct PciAddress* addr) {
	return kernsrv->pci_msi_disable(addr);
}

static inline int pci_msix_enable(struct PCIDevice* pci_dev) {
	return kernsrv->pci_msix_enable(pci_dev);
}

static inline unsigned int pci_msix_get_num_vectors(struct PCIDevice* pci_dev) {
	return kernsrv->pci_msix_get_num_vectors(pci_dev);
}

static inline void pci_msix_set_message(struct PCIDevice* pci_dev, unsigned int vec,
										const struct MSIMessage* msg) {
	return kernsrv->pci_msix_set_message(pci_dev, vec, msg);
}

static inline void pci_msix_mask(struct PCIDevice* pci_dev, unsigned int vec) {
	return kernsrv->pci_msix_mask(pci_dev, vec);
}

static inline void pci_msix_unmask(struct PCIDevice* pci_dev, unsigned int vec) {
	return kernsrv->pci_msix_unmask(pci_dev, vec);
}

static inline void pci_register_driver(const struct PCIDriver* driver) {
	return kernsrv->pci_register_driver(driver);
}

#endif
