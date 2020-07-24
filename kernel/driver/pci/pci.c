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

#include <common/x86.h>
#include <defs.h>

#include "pci-config.h"
#include "pci.h"

static const char* pci_intx_name[] = {
	[1] = "INTA", [2] = "INTB", [3] = "INTC", [4] = "INTD"};

void pci_init(void) {
	memset(pci_irq_10, 0, sizeof(pci_irq_10));
	memset(pci_irq_11, 0, sizeof(pci_irq_11));
	// display list of devices
	struct PciAddress addr;
	for (addr.bus = 0; addr.bus < PCI_BUS_MAX; addr.bus++) {
		for (addr.device = 0; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (addr.function = 0; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				uint16_t vendor = pci_read_config_reg16(&addr, PCI_CONF_VENDOR);
				if (vendor == 0xffff) {
					continue;
				}
				uint8_t intr_pin = pci_read_config_reg8(&addr, PCI_CONF_INTR_PIN);
				if (intr_pin == 0) { // No interrupt pin for this device
					cprintf("[pci] bus %d device %d func %d %x:%x class %x "
							"subclass %x progif %x\n",
							addr.bus, addr.device, addr.function, vendor,
							pci_read_config_reg16(&addr, PCI_CONF_DEVICE),
							pci_read_config_reg8(&addr, PCI_CONF_CLASS),
							pci_read_config_reg8(&addr, PCI_CONF_SUBCLASS),
							pci_read_config_reg8(&addr, PCI_CONF_PROGIF));
				} else {
					uint8_t intr_line = pci_read_config_reg8(&addr, PCI_CONF_INTR_LINE);
					cprintf("[pci] bus %d device %d func %d %x:%x class %x "
							"subclass %x progif %x %s %d\n",
							addr.bus, addr.device, addr.function, vendor,
							pci_read_config_reg16(&addr, PCI_CONF_DEVICE),
							pci_read_config_reg8(&addr, PCI_CONF_CLASS),
							pci_read_config_reg8(&addr, PCI_CONF_SUBCLASS),
							pci_read_config_reg8(&addr, PCI_CONF_PROGIF),
							pci_intx_name[intr_pin], intr_line);
					if ((intr_line == 10) || (intr_line == 11)) {
						pci_add_irq(intr_line, &addr);
					}
				}
			}
		}
	}
	// enable interrupts
	ioapicenable(10, 0);
	ioapicenable(11, 0);
}

uint8_t pci_read_config_reg8(const struct PciAddress* addr, int reg) {
	outdw(PCI_IO_CONFADD, addr->bus << PCI_IO_CONF_BUSNUM |
							  addr->device << PCI_IO_CONF_DEVNUM |
							  addr->function << PCI_IO_CONF_FUNCNUM | (reg & 0xfc) |
							  1 << PCI_IO_CONF_CONE);
	return inb(PCI_IO_CONFDATA + reg % 4);
}

uint16_t pci_read_config_reg16(const struct PciAddress* addr, int reg) {
	outdw(PCI_IO_CONFADD, addr->bus << PCI_IO_CONF_BUSNUM |
							  addr->device << PCI_IO_CONF_DEVNUM |
							  addr->function << PCI_IO_CONF_FUNCNUM | (reg & 0xfc) |
							  1 << PCI_IO_CONF_CONE);
	return inw(PCI_IO_CONFDATA + reg % 4);
}

uint32_t pci_read_config_reg32(const struct PciAddress* addr, int reg) {
	outdw(PCI_IO_CONFADD, addr->bus << PCI_IO_CONF_BUSNUM |
							  addr->device << PCI_IO_CONF_DEVNUM |
							  addr->function << PCI_IO_CONF_FUNCNUM | (reg & 0xfc) |
							  1 << PCI_IO_CONF_CONE);
	return indw(PCI_IO_CONFDATA);
}

void pci_write_config_reg8(const struct PciAddress* addr, int reg, uint8_t data) {
	outdw(PCI_IO_CONFADD, addr->bus << PCI_IO_CONF_BUSNUM |
							  addr->device << PCI_IO_CONF_DEVNUM |
							  addr->function << PCI_IO_CONF_FUNCNUM | (reg & 0xfc) |
							  1 << PCI_IO_CONF_CONE);
	outb(PCI_IO_CONFDATA + reg % 4, data);
}

void pci_write_config_reg16(const struct PciAddress* addr, int reg, uint16_t data) {
	outdw(PCI_IO_CONFADD, addr->bus << PCI_IO_CONF_BUSNUM |
							  addr->device << PCI_IO_CONF_DEVNUM |
							  addr->function << PCI_IO_CONF_FUNCNUM | (reg & 0xfc) |
							  1 << PCI_IO_CONF_CONE);
	outw(PCI_IO_CONFDATA + reg % 4, data);
}

void pci_write_config_reg32(const struct PciAddress* addr, int reg, uint32_t data) {
	outdw(PCI_IO_CONFADD, addr->bus << PCI_IO_CONF_BUSNUM |
							  addr->device << PCI_IO_CONF_DEVNUM |
							  addr->function << PCI_IO_CONF_FUNCNUM | (reg & 0xfc) |
							  1 << PCI_IO_CONF_CONE);
	outdw(PCI_IO_CONFDATA, data);
}

phyaddr_t pci_read_bar(const struct PciAddress* addr, int bar) {
	uint32_t bar_val = pci_read_config_reg32(addr, PCI_CONF_BAR + bar * 4);
	if (bar_val & 1) { // IO BAR
		return bar_val & 0xfffffffc;
	} else { // Memory BAR
		return bar_val & 0xfffffff0;
	}
}

struct PciAddress* pci_find_device(struct PciAddress* pciaddr, uint16_t vendor,
								   uint16_t device) {
	struct PciAddress addr;
	for (addr.bus = 0; addr.bus < PCI_BUS_MAX; addr.bus++) {
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
	for (addr.bus = 0; addr.bus < PCI_BUS_MAX; addr.bus++) {
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
	for (addr.bus = 0; addr.bus < PCI_BUS_MAX; addr.bus++) {
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
	for (; addr.bus < PCI_BUS_MAX; addr.bus++) {
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
	for (; addr.bus < PCI_BUS_MAX; addr.bus++) {
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
	for (; addr.bus < PCI_BUS_MAX; addr.bus++) {
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
