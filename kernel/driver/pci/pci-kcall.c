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
#include <defs.h>

#include "pci-config.h"
#include "pci.h"

struct PciKcall {
#define PCI_KCALL_OP_FIRST_ADDR 0
#define PCI_KCALL_OP_NEXT_ADDR 1
#define PCI_KCALL_OP_READ_CONFIG 2
#define PCI_KCALL_OP_DRIVER_NAME 3
#define PCI_KCALL_OP_RESOURCE 4
	unsigned int op;
	unsigned int id;
	unsigned int bus, device, function;
	void* ptr;
};

struct PciKcallResource {
	uint64_t bar_base[6], bar_size[6];
	uint64_t rombar_base, rombar_size;
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
	if (pci_host.pcie_ecam_base) {
		for (int i = 0; i < 4096; i += 4) {
			*(uint32_t*)(p->ptr + i) = pci_read_config_reg32(addr, i);
		}
	} else {
		for (int i = 0; i < 256; i += 4) {
			*(uint32_t*)(p->ptr + i) = pci_read_config_reg32(addr, i);
		}
		memset(p->ptr + 256, 0, 4096 - 256);
	}
	return 0;
}

static int pci_kcall_driver_name(struct PciKcall* p) {
	if (!pci_device_table[p->id].vendor_id) {
		return ERROR_NOT_EXIST;
	}
	char* s = p->ptr;
	if (!pci_device_table[p->id].driver) {
		s[0] = '\0';
	} else {
		safestrcpy(s, pci_device_table[p->id].driver->name, 64);
	}
	return 0;
}

static int pci_kcall_resource(struct PciKcall* p) {
	if (!pci_device_table[p->id].vendor_id) {
		return ERROR_NOT_EXIST;
	}
	const struct PciAddress* addr = &pci_device_table[p->id].addr;
	uint8_t header_type = pci_read_config_reg8(addr, PCI_CONF_HEADER_TYPE);
	int num_bars;
	if (header_type == 0x0 || header_type == 0x80) {
		num_bars = 6;
	} else {
		num_bars = 2;
	}
	struct PciKcallResource* pcires = p->ptr;
	memset(pcires, 0, sizeof(struct PciKcallResource));
	for (int i = 0; i < num_bars; i++) {
		pcires->bar_base[i] = pci_read_bar(addr, i);
		if (pcires->bar_base[i]) {
			pcires->bar_size[i] = pci_read_bar_size(addr, i);
		}
	}
	pcires->rombar_base = pci_read_rom_bar(addr);
	if (pcires->rombar_base) {
		pcires->rombar_size = pci_read_rom_bar_size(addr);
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
	case PCI_KCALL_OP_DRIVER_NAME:
		return pci_kcall_driver_name(p);
		break;
	case PCI_KCALL_OP_RESOURCE:
		return pci_kcall_resource(p);
	default:
		return ERROR_INVAILD;
	}
}
