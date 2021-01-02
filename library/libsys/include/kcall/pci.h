/*
 * PCI Subsystem user mode API
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

#ifndef _LIBSYS_KCALL_PCI_H
#define _LIBSYS_KCALL_PCI_H

#include <panicos.h>

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
#endif

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

struct PCIGenericConfigHeader {
	uint16_t vendor, device;
	uint16_t command, status;
	uint8_t revision, progif, subclass, pclass;
	uint8_t cache_line, lat_timer, header_type, bist;
	uint8_t placeholder[36];
	uint8_t capptr, res[7];
	uint8_t intr_line, intr_pin;
	uint16_t shared;
} __attribute__((packed));

struct PCIType0ConfigHeader {
	uint16_t vendor, device;
	uint16_t command, status;
	uint8_t revision, progif, subclass, pclass;
	uint8_t cache_line, lat_timer, header_type, bist;
	uint32_t bar[6];
	uint32_t cardbus_cis;
	uint16_t subsys_vendor, subsys_device;
	uint32_t rombar;
	uint8_t capptr, res[7];
	uint8_t intr_line, intr_pin, min_gnt, max_lat;
} __attribute__((packed));

struct PCIType1ConfigHeader {
	uint16_t vendor, device;
	uint16_t command, status;
	uint8_t revision, progif, subclass, pclass;
	uint8_t cache_line, lat_timer, header_type, bist;
	uint32_t bar[2];
	uint8_t pribus, secbus, subbus, seclatmr;
	uint8_t io_base, io_limit;
	uint16_t sec_status;
	uint16_t mem_base, mem_limit;
	uint16_t pref_mem_base, pref_mem_limit;
	uint32_t pref_mem_base_upper, pref_mem_limit_upper;
	uint16_t io_base_upper, io_limit_upper;
	uint8_t capptr, res[7];
	uint8_t intr_line, intr_pin;
	uint16_t bridge_control;
} __attribute__((packed));

struct PCIAddr {
	unsigned int bus, device, function;
};

static inline unsigned int pci_get_first_addr(struct PCIAddr* addr) {
	struct PciKcall pcikcall;
	pcikcall.op = PCI_KCALL_OP_FIRST_ADDR;
	if (kcall("pci", (unsigned int)&pcikcall)) {
		addr->bus = pcikcall.bus;
		addr->device = pcikcall.device;
		addr->function = pcikcall.function;
		return pcikcall.id;
	} else {
		return 0xffffffff;
	}
}

static inline unsigned int pci_get_next_addr(unsigned int id, struct PCIAddr* addr) {
	struct PciKcall pcikcall;
	pcikcall.op = PCI_KCALL_OP_NEXT_ADDR;
	pcikcall.id = id;
	if (kcall("pci", (unsigned int)&pcikcall)) {
		addr->bus = pcikcall.bus;
		addr->device = pcikcall.device;
		addr->function = pcikcall.function;
		return pcikcall.id;
	} else {
		return 0;
	}
}

static inline void pci_read_config(unsigned int id, void* buf) {
	struct PciKcall pcikcall;
	pcikcall.op = PCI_KCALL_OP_READ_CONFIG;
	pcikcall.id = id;
	pcikcall.ptr = buf;
	kcall("pci", (unsigned int)&pcikcall);
}

static inline void pci_get_driver_name(unsigned int id, char* buf) {
	struct PciKcall pcikcall;
	pcikcall.op = PCI_KCALL_OP_DRIVER_NAME;
	pcikcall.id = id;
	pcikcall.ptr = buf;
	kcall("pci", (unsigned int)&pcikcall);
}

static inline void pci_get_resource(unsigned int id, struct PciKcallResource* pcires) {
	struct PciKcall pcikcall;
	pcikcall.op = PCI_KCALL_OP_RESOURCE;
	pcikcall.id = id;
	pcikcall.ptr = pcires;
	kcall("pci", (unsigned int)&pcikcall);
}

#endif
