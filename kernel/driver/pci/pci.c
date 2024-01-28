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
#include <driver/x86/ioapic.h>
#include <memlayout.h>
#include <proc/kcall.h>

#include "pci-config.h"
#include "pci.h"

static const char *pci_intx_name[] = {[1] = "INTA", [2] = "INTB", [3] = "INTC", [4] = "INTD"};

void pci_init(void) {
	memset(pci_device_table, 0, sizeof(pci_device_table));
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
							addr.bus, addr.device, addr.function, vendor_id, device_id, class,
							subclass, progif);
					pci_add_device(&addr, vendor_id, device_id, class, subclass, progif, 0);
				} else {
					uint8_t intr_line = pci_read_config_reg8(&addr, PCI_CONF_INTR_LINE);
					cprintf("[pci] bus %d device %d func %d %x:%x class %x "
							"subclass %x progif %x %s %d\n",
							addr.bus, addr.device, addr.function, vendor_id, device_id, class,
							subclass, progif, pci_intx_name[intr_pin], intr_line);
					pci_add_device(&addr, vendor_id, device_id, class, subclass, progif, intr_line);
				}
			}
		}
	}
// enable interrupts
#ifndef __riscv
	ioapic_enable(5, 0, IOAPIC_LEVEL_TRIGGER, IOAPIC_ACTIVE_HIGH);
	ioapic_enable(9, 0, IOAPIC_LEVEL_TRIGGER, IOAPIC_ACTIVE_HIGH);
	ioapic_enable(10, 0, IOAPIC_LEVEL_TRIGGER, IOAPIC_ACTIVE_HIGH);
	ioapic_enable(11, 0, IOAPIC_LEVEL_TRIGGER, IOAPIC_ACTIVE_HIGH);
#endif
	kcall_set("pci", pci_kcall_handler);
}

uint8_t pci_read_config_reg8(const struct PciAddress *addr, int reg) {
	return pci_host.read8(addr, reg);
}

uint16_t pci_read_config_reg16(const struct PciAddress *addr, int reg) {
	return pci_host.read16(addr, reg);
}

uint32_t pci_read_config_reg32(const struct PciAddress *addr, int reg) {
	return pci_host.read32(addr, reg);
}

void pci_write_config_reg8(const struct PciAddress *addr, int reg, uint8_t data) {
	return pci_host.write8(addr, reg, data);
}

void pci_write_config_reg16(const struct PciAddress *addr, int reg, uint16_t data) {
	return pci_host.write16(addr, reg, data);
}

void pci_write_config_reg32(const struct PciAddress *addr, int reg, uint32_t data) {
	return pci_host.write32(addr, reg, data);
}

phyaddr_t pci_read_bar(const struct PciAddress *addr, int bar) {
	uint32_t bar_val = pci_read_config_reg32(addr, PCI_CONF_BAR + bar * 4);
	if (bar_val & 1) { // IO BAR
		return bar_val & 0xfffffffc;
	} else { // Memory BAR
		if ((bar_val & 6) == 4) { // 64 bits
			if (pci_read_config_reg32(addr, PCI_CONF_BAR + bar * 4 + 4) != 0) {
				return 0; // for BAR above 4G, pretend not exist
			}
		}
		return bar_val & 0xfffffff0;
	}
}

// TODO: Support 64-bits BAR
size_t pci_read_bar_size(const struct PciAddress *addr, int bar) {
	size_t bar_size;
	uint32_t oldbar = pci_read_config_reg32(addr, PCI_CONF_BAR + bar * 4);
	pci_write_config_reg32(addr, PCI_CONF_BAR + bar * 4, 0xffffffff);
	if (oldbar & 1) { // IO BAR
		bar_size = ~(pci_read_config_reg32(addr, PCI_CONF_BAR + bar * 4) & 0xfffffffc) + 1;
	} else { // Memory BAR
		bar_size = ~(pci_read_config_reg32(addr, PCI_CONF_BAR + bar * 4) & 0xfffffff0) + 1;
	}
	pci_write_config_reg32(addr, PCI_CONF_BAR + bar * 4, oldbar);
	return bar_size;
}

phyaddr_t pci_read_rom_bar(const struct PciAddress *addr) {
	uint32_t rombar = pci_read_config_reg32(addr, PCI_CONF_ROM_BAR);
	if ((rombar & 1) == 0) { // enable if necessary
		pci_write_config_reg32(addr, PCI_CONF_ROM_BAR, rombar | 1);
	}
	return rombar & 0xfffff800;
}

size_t pci_read_rom_bar_size(const struct PciAddress *addr) {
	uint32_t rombar = pci_read_config_reg32(addr, PCI_CONF_ROM_BAR);
	pci_write_config_reg32(addr, PCI_CONF_ROM_BAR, 0xfffff800);
	uint32_t val = pci_read_config_reg32(addr, PCI_CONF_ROM_BAR);
	pci_write_config_reg32(addr, PCI_CONF_ROM_BAR, rombar);
	return ~(val & 0xfffff800) + 1;
}

void pci_enable_bus_mastering(const struct PciAddress *addr) {
	uint16_t pcicmd = pci_read_config_reg16(addr, PCI_CONF_COMMAND);
	pcicmd |= PCI_CONTROL_BUS_MASTER;
	pci_write_config_reg16(addr, PCI_CONF_COMMAND, pcicmd);
}

int pci_find_capability(const struct PciAddress *addr, uint8_t cap_id) {
	int capptr = pci_read_config_reg8(addr, PCI_CONF_CAP_PTR);
	while (capptr) {
		if (pci_read_config_reg8(addr, capptr) == cap_id) {
			return capptr;
		}
		capptr = pci_read_config_reg8(addr, capptr + 1);
	}
	return 0;
}

struct PciAddress *pci_find_device(struct PciAddress *pciaddr, uint16_t vendor, uint16_t device) {
	struct PciAddress addr;
	for (addr.bus = 0; addr.bus < pci_host.bus_num; addr.bus++) {
		for (addr.device = 0; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (addr.function = 0; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				if ((vendor == pci_read_config_reg16(&addr, PCI_CONF_VENDOR))
					&& (device == pci_read_config_reg16(&addr, PCI_CONF_DEVICE))) {
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

struct PciAddress *pci_find_class(struct PciAddress *pciaddr, uint8_t class, uint8_t subclass) {
	struct PciAddress addr;
	for (addr.bus = 0; addr.bus < pci_host.bus_num; addr.bus++) {
		for (addr.device = 0; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (addr.function = 0; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				if ((class == pci_read_config_reg8(&addr, PCI_CONF_CLASS))
					&& (subclass == pci_read_config_reg8(&addr, PCI_CONF_SUBCLASS))) {
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

struct PciAddress *pci_find_progif(struct PciAddress *pciaddr, uint8_t class, uint8_t subclass,
								   uint8_t progif) {
	struct PciAddress addr;
	for (addr.bus = 0; addr.bus < pci_host.bus_num; addr.bus++) {
		for (addr.device = 0; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (addr.function = 0; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				if ((class == pci_read_config_reg8(&addr, PCI_CONF_CLASS))
					&& (subclass == pci_read_config_reg8(&addr, PCI_CONF_SUBCLASS))
					&& (progif == pci_read_config_reg8(&addr, PCI_CONF_PROGIF))) {
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

struct PciAddress *pci_next_device(struct PciAddress *pciaddr, uint16_t vendor, uint16_t device) {
	struct PciAddress addr;
	addr.bus = pciaddr->bus;
	addr.device = pciaddr->device;
	addr.function = pciaddr->function + 1;
	for (; addr.bus < pci_host.bus_num; addr.bus++) {
		for (; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				if ((vendor == pci_read_config_reg16(&addr, PCI_CONF_VENDOR))
					&& (device == pci_read_config_reg16(&addr, PCI_CONF_DEVICE))) {
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

struct PciAddress *pci_next_class(struct PciAddress *pciaddr, uint8_t class, uint8_t subclass) {
	struct PciAddress addr;
	addr.bus = pciaddr->bus;
	addr.device = pciaddr->device;
	addr.function = pciaddr->function + 1;
	for (; addr.bus < pci_host.bus_num; addr.bus++) {
		for (; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				if ((class == pci_read_config_reg8(&addr, PCI_CONF_CLASS))
					&& (subclass == pci_read_config_reg8(&addr, PCI_CONF_SUBCLASS))) {
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

struct PciAddress *pci_next_progif(struct PciAddress *pciaddr, uint8_t class, uint8_t subclass,
								   uint8_t progif) {
	struct PciAddress addr;
	addr.bus = pciaddr->bus;
	addr.device = pciaddr->device;
	addr.function = pciaddr->function + 1;
	for (; addr.bus < pci_host.bus_num; addr.bus++) {
		for (; addr.device < PCI_DEVICE_MAX; addr.device++) {
			for (; addr.function < PCI_FUNCTION_MAX; addr.function++) {
				if ((class == pci_read_config_reg8(&addr, PCI_CONF_CLASS))
					&& (subclass == pci_read_config_reg8(&addr, PCI_CONF_SUBCLASS))
					&& (progif == pci_read_config_reg8(&addr, PCI_CONF_PROGIF))) {
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

void pci_enable_device(const struct PciAddress *addr) {
	uint16_t pcicmd = pci_read_config_reg16(addr, PCI_CONF_COMMAND);
	pcicmd |= (PCI_CONTROL_IO_SPACE | PCI_CONTROL_MEMORY_SPACE);
	pci_write_config_reg16(addr, PCI_CONF_COMMAND, pcicmd);
}
