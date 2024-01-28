/*
 * PCIe ECAM on Intel chipsets
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

#include <defs.h>
#include <driver/pci/pci.h>

static uint8_t ecam_read_config_reg8(const struct PciAddress *addr, int reg) {
	return *(volatile uint8_t *)(pci_host.pcie_ecam_base
								 + ((addr->bus << 20) | (addr->device << 15)
									| (addr->function << 12) | reg));
}

static uint16_t ecam_read_config_reg16(const struct PciAddress *addr, int reg) {
	return *(volatile uint16_t *)(pci_host.pcie_ecam_base
								  + ((addr->bus << 20) | (addr->device << 15)
									 | (addr->function << 12) | reg));
}

static uint32_t ecam_read_config_reg32(const struct PciAddress *addr, int reg) {
	return *(volatile uint32_t *)(pci_host.pcie_ecam_base
								  + ((addr->bus << 20) | (addr->device << 15)
									 | (addr->function << 12) | reg));
}

static void ecam_write_config_reg8(const struct PciAddress *addr, int reg, uint8_t data) {
	*(volatile uint8_t *)(pci_host.pcie_ecam_base
						  + ((addr->bus << 20) | (addr->device << 15) | (addr->function << 12)
							 | reg))
		= data;
}

static void ecam_write_config_reg16(const struct PciAddress *addr, int reg, uint16_t data) {
	*(volatile uint16_t *)(pci_host.pcie_ecam_base
						   + ((addr->bus << 20) | (addr->device << 15) | (addr->function << 12)
							  | reg))
		= data;
}

static void ecam_write_config_reg32(const struct PciAddress *addr, int reg, uint32_t data) {
	*(volatile uint32_t *)(pci_host.pcie_ecam_base
						   + ((addr->bus << 20) | (addr->device << 15) | (addr->function << 12)
							  | reg))
		= data;
}

int ecam_init(void) {
	phyaddr_t ecam_base = 0x30000000;
	cprintf("[pci] PCIe ECAM addr %x\n", ecam_base);
	// write to pci_host
	pci_host.read8 = ecam_read_config_reg8;
	pci_host.read16 = ecam_read_config_reg16;
	pci_host.read32 = ecam_read_config_reg32;
	pci_host.write8 = ecam_write_config_reg8;
	pci_host.write16 = ecam_write_config_reg16;
	pci_host.write32 = ecam_write_config_reg32;
	pci_host.bus_num = 256;
	pci_host.pcie_ecam_base = map_mmio_region(ecam_base, 0x10000000);
	return 0;
}

struct PciHost pci_host;
