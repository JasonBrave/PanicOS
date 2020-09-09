/*
 * PCI Bus user API
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

#include <common/errorcode.h>

#include "pci.h"

struct PciKcall {
#define PCI_KCALL_OP_FIRST_ADDR 0
#define PCI_KCALL_OP_NEXT_ADDR 1
#define PCI_KCALL_OP_READ_CONFIG 2
	unsigned int op;
	unsigned int id;
	unsigned int bus, device, function;
	void* ptr;
};

static int pci_kcall_first_addr(struct PciKcall* p) {
	for (int i = 0; i < PCI_DEVICE_TABLE_SIZE; i++) {
		if (pci_device_table[i].vendor_id) {
			p->id = i;
			p->bus = pci_device_table[i].addr.bus;
			p->device = pci_device_table[i].addr.device;
			p->function = pci_device_table[i].addr.function;
			return 1;
		}
	}
	return 0;
}

static int pci_kcall_next_addr(struct PciKcall* p) {
	for (int i = p->id + 1; i < PCI_DEVICE_TABLE_SIZE; i++) {
		if (pci_device_table[i].vendor_id) {
			p->id = i;
			p->bus = pci_device_table[i].addr.bus;
			p->device = pci_device_table[i].addr.device;
			p->function = pci_device_table[i].addr.function;
			return 1;
		}
	}
	return 0;
}

static int pci_kcall_read_config(struct PciKcall* p) {
	if (!pci_device_table[p->id].vendor_id) {
		return ERROR_NOT_EXIST;
	}
	const struct PciAddress* addr = &pci_device_table[p->id].addr;
	for (int i = 0; i < 256; i += 4) {
		*(uint32_t*)(p->ptr + i) = pci_read_config_reg32(addr, i);
	}
	return 0;
}

int pci_kcall_handler(unsigned int t) {
	struct PciKcall* p = (void*)t;
	switch (p->op) {
	case PCI_KCALL_OP_FIRST_ADDR:
		return pci_kcall_first_addr(p);
		break;
	case PCI_KCALL_OP_NEXT_ADDR:
		return pci_kcall_next_addr(p);
		break;
	case PCI_KCALL_OP_READ_CONFIG:
		return pci_kcall_read_config(p);
		break;
	default:
		return ERROR_INVAILD;
	}
}
