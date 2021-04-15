/*
 * Legacy PCI Configuration Space access
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

#include <common/x86.h>
#include <driver/pci/pci.h>

#define PCI_IO_CONFADD 0xcf8
#define PCI_IO_CONFDATA 0xcfc

#define PCI_IO_CONF_CONE 31
#define PCI_IO_CONF_BUSNUM 16
#define PCI_IO_CONF_DEVNUM 11
#define PCI_IO_CONF_FUNCNUM 8
#define PCI_IO_CONF_REGNUM 2

static uint8_t pci_legacy_read_config_reg8(const struct PciAddress* addr, int reg) {
	outdw(PCI_IO_CONFADD, addr->bus << PCI_IO_CONF_BUSNUM | addr->device << PCI_IO_CONF_DEVNUM |
							  addr->function << PCI_IO_CONF_FUNCNUM | (reg & 0xfc) |
							  1 << PCI_IO_CONF_CONE);
	return inb(PCI_IO_CONFDATA + reg % 4);
}

static uint16_t pci_legacy_read_config_reg16(const struct PciAddress* addr, int reg) {
	outdw(PCI_IO_CONFADD, addr->bus << PCI_IO_CONF_BUSNUM | addr->device << PCI_IO_CONF_DEVNUM |
							  addr->function << PCI_IO_CONF_FUNCNUM | (reg & 0xfc) |
							  1 << PCI_IO_CONF_CONE);
	return inw(PCI_IO_CONFDATA + reg % 4);
}

static uint32_t pci_legacy_read_config_reg32(const struct PciAddress* addr, int reg) {
	outdw(PCI_IO_CONFADD, addr->bus << PCI_IO_CONF_BUSNUM | addr->device << PCI_IO_CONF_DEVNUM |
							  addr->function << PCI_IO_CONF_FUNCNUM | (reg & 0xfc) |
							  1 << PCI_IO_CONF_CONE);
	return indw(PCI_IO_CONFDATA);
}

static void pci_legacy_write_config_reg8(const struct PciAddress* addr, int reg, uint8_t data) {
	outdw(PCI_IO_CONFADD, addr->bus << PCI_IO_CONF_BUSNUM | addr->device << PCI_IO_CONF_DEVNUM |
							  addr->function << PCI_IO_CONF_FUNCNUM | (reg & 0xfc) |
							  1 << PCI_IO_CONF_CONE);
	outb(PCI_IO_CONFDATA + reg % 4, data);
}

static void pci_legacy_write_config_reg16(const struct PciAddress* addr, int reg, uint16_t data) {
	outdw(PCI_IO_CONFADD, addr->bus << PCI_IO_CONF_BUSNUM | addr->device << PCI_IO_CONF_DEVNUM |
							  addr->function << PCI_IO_CONF_FUNCNUM | (reg & 0xfc) |
							  1 << PCI_IO_CONF_CONE);
	outw(PCI_IO_CONFDATA + reg % 4, data);
}

static void pci_legacy_write_config_reg32(const struct PciAddress* addr, int reg, uint32_t data) {
	outdw(PCI_IO_CONFADD, addr->bus << PCI_IO_CONF_BUSNUM | addr->device << PCI_IO_CONF_DEVNUM |
							  addr->function << PCI_IO_CONF_FUNCNUM | (reg & 0xfc) |
							  1 << PCI_IO_CONF_CONE);
	outdw(PCI_IO_CONFDATA, data);
}

struct PciHost pci_host = {
	.read8 = pci_legacy_read_config_reg8,
	.read16 = pci_legacy_read_config_reg16,
	.read32 = pci_legacy_read_config_reg32,
	.write8 = pci_legacy_write_config_reg8,
	.write16 = pci_legacy_write_config_reg16,
	.write32 = pci_legacy_write_config_reg32,
	.bus_num = 256,
	.pcie_ecam_base = 0, // not PCIe
};
