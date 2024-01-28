/*
 * PCI Device Driver management
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

#include "pci.h"

struct PCIDevice pci_device_table[PCI_DEVICE_TABLE_SIZE];

void pci_add_device(const struct PciAddress *addr, uint16_t vendor_id, uint16_t device_id,
					uint8_t class, uint8_t subclass, uint8_t progif, uint8_t irq) {
	for (int i = 0; i < PCI_DEVICE_TABLE_SIZE; i++) {
		if (!pci_device_table[i].vendor_id) {
			pci_device_table[i].addr = *addr;
			pci_device_table[i].vendor_id = vendor_id;
			pci_device_table[i].device_id = device_id;
			pci_device_table[i].class = class;
			pci_device_table[i].subclass = subclass;
			pci_device_table[i].progif = progif;
			pci_device_table[i].irq = irq;
			return;
		}
	}
	panic("too many pci devices");
}

void pci_register_driver(const struct PCIDriver *driver) {
	if (driver->match_table) {
		const struct PCIDeviceID *id = driver->match_table;
		while (id->vendor_id) {
			for (int i = 0; i < PCI_DEVICE_TABLE_SIZE; i++) {
				if (!pci_device_table[i].driver && pci_device_table[i].vendor_id == id->vendor_id
					&& pci_device_table[i].device_id == id->device_id) {
					pci_device_table[i].driver = driver;
					pci_enable_device(&pci_device_table[i].addr);
					driver->init(&pci_device_table[i]);
				}
			}
			id++;
		}
	}
	if (driver->class_type && (driver->class_type & 0xff) == 0xff) { // class driver
		for (int i = 0; i < PCI_DEVICE_TABLE_SIZE; i++) {
			if (!pci_device_table[i].driver
				&& pci_device_table[i].class == (driver->class_type >> 16 & 0xff)
				&& pci_device_table[i].subclass == (driver->class_type >> 8 & 0xff)) {
				pci_device_table[i].driver = driver;
				pci_enable_device(&pci_device_table[i].addr);
				driver->init(&pci_device_table[i]);
			}
		}
	} else if (driver->class_type) { // progif driver
		for (int i = 0; i < PCI_DEVICE_TABLE_SIZE; i++) {
			if (!pci_device_table[i].driver
				&& pci_device_table[i].class == (driver->class_type >> 16 & 0xff)
				&& pci_device_table[i].subclass == (driver->class_type >> 8 & 0xff)
				&& pci_device_table[i].progif == (driver->class_type & 0xff)) {
				pci_device_table[i].driver = driver;
				pci_enable_device(&pci_device_table[i].addr);
				driver->init(&pci_device_table[i]);
			}
		}
	}
}

void pci_print_devices(void) {
	for (int i = 0; i < PCI_DEVICE_TABLE_SIZE; i++) {
		if (!pci_device_table[i].vendor_id) {
			continue;
		}
		if (pci_device_table[i].driver) {
			cprintf("PCI %d:%d.%d %x:%x class %x "
					"subclass %x progif %x irq %d driver %s\n",
					pci_device_table[i].addr.bus, pci_device_table[i].addr.device,
					pci_device_table[i].addr.function, pci_device_table[i].vendor_id,
					pci_device_table[i].device_id, pci_device_table[i].class,
					pci_device_table[i].subclass, pci_device_table[i].progif,
					pci_device_table[i].irq, pci_device_table[i].driver->name);
		} else {
			cprintf("PCI %d:%d.%d %x:%x class %x "
					"subclass %x progif %x irq %d driver <none>\n",
					pci_device_table[i].addr.bus, pci_device_table[i].addr.device,
					pci_device_table[i].addr.function, pci_device_table[i].vendor_id,
					pci_device_table[i].device_id, pci_device_table[i].class,
					pci_device_table[i].subclass, pci_device_table[i].progif,
					pci_device_table[i].irq);
		}
	}
}
