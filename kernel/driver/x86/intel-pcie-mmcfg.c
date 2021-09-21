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

static uint8_t intel_mmcfg_read_config_reg8(const struct PciAddress* addr, int reg) {
	return *(volatile uint8_t*)(pci_host.pcie_ecam_base + ((addr->bus << 20) |
														   (addr->device << 15) |
														   (addr->function << 12) | reg));
}

static uint16_t intel_mmcfg_read_config_reg16(const struct PciAddress* addr, int reg) {
	return *(volatile uint16_t*)(pci_host.pcie_ecam_base + ((addr->bus << 20) |
															(addr->device << 15) |
															(addr->function << 12) | reg));
}

static uint32_t intel_mmcfg_read_config_reg32(const struct PciAddress* addr, int reg) {
	return *(volatile uint32_t*)(pci_host.pcie_ecam_base + ((addr->bus << 20) |
															(addr->device << 15) |
															(addr->function << 12) | reg));
}

static void intel_mmcfg_write_config_reg8(const struct PciAddress* addr, int reg, uint8_t data) {
	*(volatile uint8_t*)(pci_host.pcie_ecam_base + ((addr->bus << 20) | (addr->device << 15) |
													(addr->function << 12) | reg)) = data;
}

static void intel_mmcfg_write_config_reg16(const struct PciAddress* addr, int reg, uint16_t data) {
	*(volatile uint16_t*)(pci_host.pcie_ecam_base + ((addr->bus << 20) | (addr->device << 15) |
													 (addr->function << 12) | reg)) = data;
}

static void intel_mmcfg_write_config_reg32(const struct PciAddress* addr, int reg, uint32_t data) {
	*(volatile uint32_t*)(pci_host.pcie_ecam_base + ((addr->bus << 20) | (addr->device << 15) |
													 (addr->function << 12) | reg)) = data;
}

int intel_pcie_mmcfg_init(const struct PciAddress* host_bridge_addr) {
	// ensure this is an Intel host bridge
	if (pci_read_config_reg16(host_bridge_addr, 0x0) != 0x8086) {
		return -1;
	}
	// read PCIEXBAR
	uint64_t pciexbar = pci_read_config_reg32(host_bridge_addr, 0x64);
	pciexbar = pciexbar << 32;
	pciexbar |= pci_read_config_reg32(host_bridge_addr, 0x60);
	// check no reserved bytes are set
	if (!pciexbar) {
		return -1;
	}
	if (pciexbar & 0xffffff8000000000) {
		return -1;
	}
	if (pciexbar & 0x3fffff0) {
		return -1;
	}
	if (!(pciexbar & 1)) {
		cprintf("[pci] PCIEXBAR %x disabled\n", pciexbar);
		return -1;
	}
	// number of bus
	int num_bus;
	switch (pciexbar & 6) {
	case 0 << 1:
		num_bus = 256;
		break;
	case 1 << 1:
		num_bus = 128;
		break;
	case 2 << 1:
		num_bus = 64;
		break;
	case 3 << 1:
		num_bus = 512;
		break;
	case 4 << 1:
		num_bus = 1024;
		break;
	case 5 << 1:
		num_bus = 2048;
		break;
	case 6 << 1:
		num_bus = 4096;
		break;
	default:
		panic("invaild PCIEXBAR");
	}
	// ECAM base address
	if ((pciexbar & 0x7ffc000000) > 0xe0000000) {
		cprintf("[pci] PCIEXBAR too high %llx\n", pciexbar);
		return -1;
	}
	if ((pciexbar & 0x7ffc000000) < 0xb0000000) {
		cprintf("[pci] PCIEXBAR too low %llx\n", pciexbar);
		return -1;
	}
	phyaddr_t ecam_base = pciexbar & 0xfc000000;
	cprintf("[pci] PCIe ECAM addr %x bus %d\n", ecam_base, num_bus);
	// write to pci_host
	pci_host.read8 = intel_mmcfg_read_config_reg8;
	pci_host.read16 = intel_mmcfg_read_config_reg16;
	pci_host.read32 = intel_mmcfg_read_config_reg32;
	pci_host.write8 = intel_mmcfg_write_config_reg8;
	pci_host.write16 = intel_mmcfg_write_config_reg16;
	pci_host.write32 = intel_mmcfg_write_config_reg32;
	pci_host.bus_num = num_bus;
	pci_host.pcie_ecam_base = map_mmio_region(ecam_base, num_bus * 0x100000);
	return 0;
}
