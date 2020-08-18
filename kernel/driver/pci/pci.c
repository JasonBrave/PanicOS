/*
 * PCI Local Bus support
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

#include "pci-config.h"
#include "pci.h"

static const char* pci_intx_name[] = {
	[1] = "INTA", [2] = "INTB", [3] = "INTC", [4] = "INTD"};

void pci_init(void) {
	memset(pci_msi_vector, 0, sizeof(pci_msi_vector));
	memset(pci_device_table, 0, sizeof(pci_device_table));
	// PCIe ECAM
	const struct PciAddress pci_host_bridge_addr = {
		.bus = 0, .device = 0, .function = 0};
	if ((pci_read_config_reg16(&pci_host_bridge_addr, PCI_CONF_VENDOR) == 0x8086) &&
		(pci_read_config_reg16(&pci_host_bridge_addr, PCI_CONF_DEVICE) ==
		 0x29c0)) { // Q35 and P35 chipset
		intel_pcie_mmcfg_init(&pci_host_bridge_addr);
	}
	// display list of devices
	struct PciAddress addr;
	for (addr.bus = 0; addr.bus < pci_host.bus_num; addr.bus++) {
		for (addr.device = 0; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (addr.function = 0; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				uint16_t vendor_id = pci_read_config_reg16(&addr, PCI_CONF_VENDOR);
				if (vendor_id == 0xffff) {
					continue;
				}
				uint8_t intr_pin = pci_read_config_reg8(&addr, PCI_CONF_INTR_PIN);
				uint16_t device_id = pci_read_config_reg16(&addr, PCI_CONF_DEVICE);
				uint8_t class = pci_read_config_reg8(&addr, PCI_CONF_CLASS);
				uint8_t subclass = pci_read_config_reg8(&addr, PCI_CONF_SUBCLASS);
				uint8_t progif = pci_read_config_reg8(&addr, PCI_CONF_PROGIF);
				if (intr_pin == 0) { // No interrupt pin for this device
					cprintf("[pci] bus %d device %d func %d %x:%x class %x "
							"subclass %x progif %x\n",
							addr.bus, addr.device, addr.function, vendor_id, device_id,
							class, subclass, progif);
					pci_add_device(&addr, vendor_id, device_id, class, subclass, progif,
								   0);
				} else {
					uint8_t intr_line = pci_read_config_reg8(&addr, PCI_CONF_INTR_LINE);
					cprintf("[pci] bus %d device %d func %d %x:%x class %x "
							"subclass %x progif %x %s %d\n",
							addr.bus, addr.device, addr.function, vendor_id, device_id,
							class, subclass, progif, pci_intx_name[intr_pin],
							intr_line);
					pci_add_device(&addr, vendor_id, device_id, class, subclass, progif,
								   intr_line);
				}
			}
		}
	}
	// enable interrupts
	ioapicenable(5, 0);
	ioapicenable(9, 0);
	ioapicenable(10, 0);
	ioapicenable(11, 0);
}

uint8_t pci_read_config_reg8(const struct PciAddress* addr, int reg) {
	return pci_host.read8(addr, reg);
}

uint16_t pci_read_config_reg16(const struct PciAddress* addr, int reg) {
	return pci_host.read16(addr, reg);
}

uint32_t pci_read_config_reg32(const struct PciAddress* addr, int reg) {
	return pci_host.read32(addr, reg);
}

void pci_write_config_reg8(const struct PciAddress* addr, int reg, uint8_t data) {
	return pci_host.write8(addr, reg, data);
}

void pci_write_config_reg16(const struct PciAddress* addr, int reg, uint16_t data) {
	return pci_host.write16(addr, reg, data);
}

void pci_write_config_reg32(const struct PciAddress* addr, int reg, uint32_t data) {
	return pci_host.write32(addr, reg, data);
}

phyaddr_t pci_read_bar(const struct PciAddress* addr, int bar) {
	uint32_t bar_val = pci_read_config_reg32(addr, PCI_CONF_BAR + bar * 4);
	if (bar_val & 1) { // IO BAR
		return bar_val & 0xfffffffc;
	} else { // Memory BAR
		return bar_val & 0xfffffff0;
	}
}

void pci_enable_bus_mastering(const struct PciAddress* addr) {
	uint16_t pcicmd = pci_read_config_reg16(addr, PCI_CONF_COMMAND);
	pcicmd |= PCI_CONTROL_BUS_MASTER;
	pci_write_config_reg16(addr, PCI_CONF_COMMAND, pcicmd);
}

int pci_find_capability(const struct PciAddress* addr, uint8_t cap_id) {
	int capptr = pci_read_config_reg8(addr, PCI_CONF_CAP_PTR);
	while (capptr) {
		if (pci_read_config_reg8(addr, capptr) == cap_id) {
			return capptr;
		}
		capptr = pci_read_config_reg8(addr, capptr + 1);
	}
	return 0;
}

struct PciAddress* pci_find_device(struct PciAddress* pciaddr, uint16_t vendor,
								   uint16_t device) {
	struct PciAddress addr;
	for (addr.bus = 0; addr.bus < pci_host.bus_num; addr.bus++) {
		for (addr.device = 0; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (addr.function = 0; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				if ((vendor == pci_read_config_reg16(&addr, PCI_CONF_VENDOR)) &&
					(device == pci_read_config_reg16(&addr, PCI_CONF_DEVICE))) {
					pciaddr->bus = addr.bus;
					pciaddr->device = addr.device;
					pciaddr->function = addr.function;
					return pciaddr;
				}
			}
		}
	}
	return 0;
}

struct PciAddress* pci_find_class(struct PciAddress* pciaddr, uint8_t class,
								  uint8_t subclass) {
	struct PciAddress addr;
	for (addr.bus = 0; addr.bus < pci_host.bus_num; addr.bus++) {
		for (addr.device = 0; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (addr.function = 0; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				if ((class == pci_read_config_reg8(&addr, PCI_CONF_CLASS)) &&
					(subclass == pci_read_config_reg8(&addr, PCI_CONF_SUBCLASS))) {
					pciaddr->bus = addr.bus;
					pciaddr->device = addr.device;
					pciaddr->function = addr.function;
					return pciaddr;
				}
			}
		}
	}
	return 0;
}

struct PciAddress* pci_find_progif(struct PciAddress* pciaddr, uint8_t class,
								   uint8_t subclass, uint8_t progif) {
	struct PciAddress addr;
	for (addr.bus = 0; addr.bus < pci_host.bus_num; addr.bus++) {
		for (addr.device = 0; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (addr.function = 0; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				if ((class == pci_read_config_reg8(&addr, PCI_CONF_CLASS)) &&
					(subclass == pci_read_config_reg8(&addr, PCI_CONF_SUBCLASS)) &&
					(progif == pci_read_config_reg8(&addr, PCI_CONF_PROGIF))) {
					pciaddr->bus = addr.bus;
					pciaddr->device = addr.device;
					pciaddr->function = addr.function;
					return pciaddr;
				}
			}
		}
	}
	return 0;
}

struct PciAddress* pci_next_device(struct PciAddress* pciaddr, uint16_t vendor,
								   uint16_t device) {
	struct PciAddress addr;
	addr.bus = pciaddr->bus;
	addr.device = pciaddr->device;
	addr.function = pciaddr->function + 1;
	for (; addr.bus < pci_host.bus_num; addr.bus++) {
		for (; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				if ((vendor == pci_read_config_reg16(&addr, PCI_CONF_VENDOR)) &&
					(device == pci_read_config_reg16(&addr, PCI_CONF_DEVICE))) {
					pciaddr->bus = addr.bus;
					pciaddr->device = addr.device;
					pciaddr->function = addr.function;
					return pciaddr;
				}
			}
			addr.function = 0;
		}
		addr.device = 0;
	}
	return 0;
}

struct PciAddress* pci_next_class(struct PciAddress* pciaddr, uint8_t class,
								  uint8_t subclass) {
	struct PciAddress addr;
	addr.bus = pciaddr->bus;
	addr.device = pciaddr->device;
	addr.function = pciaddr->function + 1;
	for (; addr.bus < pci_host.bus_num; addr.bus++) {
		for (; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				if ((class == pci_read_config_reg8(&addr, PCI_CONF_CLASS)) &&
					(subclass == pci_read_config_reg8(&addr, PCI_CONF_SUBCLASS))) {
					pciaddr->bus = addr.bus;
					pciaddr->device = addr.device;
					pciaddr->function = addr.function;
					return pciaddr;
				}
			}
			addr.function = 0;
		}
		addr.device = 0;
	}
	return 0;
}

struct PciAddress* pci_next_progif(struct PciAddress* pciaddr, uint8_t class,
								   uint8_t subclass, uint8_t progif) {
	struct PciAddress addr;
	addr.bus = pciaddr->bus;
	addr.device = pciaddr->device;
	addr.function = pciaddr->function + 1;
	for (; addr.bus < pci_host.bus_num; addr.bus++) {
		for (; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				if ((class == pci_read_config_reg8(&addr, PCI_CONF_CLASS)) &&
					(subclass == pci_read_config_reg8(&addr, PCI_CONF_SUBCLASS)) &&
					(progif == pci_read_config_reg8(&addr, PCI_CONF_PROGIF))) {
					pciaddr->bus = addr.bus;
					pciaddr->device = addr.device;
					pciaddr->function = addr.function;
					return pciaddr;
				}
			}
			addr.function = 0;
		}
		addr.device = 0;
	}
	return 0;
}
