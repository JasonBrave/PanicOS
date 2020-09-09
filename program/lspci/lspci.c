/*
 * lspci program
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

#include <panicos.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct PciKcall {
#define PCI_KCALL_OP_FIRST_ADDR 0
#define PCI_KCALL_OP_NEXT_ADDR 1
#define PCI_KCALL_OP_READ_CONFIG 2
	unsigned int op;
	unsigned int id;
	unsigned int bus, device, function;
	void* ptr;
};

struct PCIConfigHeader {
	uint16_t vendor, device;
} __attribute__((packed));

void lspci_print_device(unsigned int bus, unsigned int device, unsigned int function,
						void* cfg_space) {
	struct PCIConfigHeader* cfg = cfg_space;
	printf("%x:%x.%x Vendor %x Device %x\n", bus, device, function, cfg->vendor,
		   cfg->device);
}

int main() {
	void* cfg_space = malloc(4096);
	struct PciKcall pcikcall;
	pcikcall.op = PCI_KCALL_OP_FIRST_ADDR;
	if (kcall("pci", (unsigned int)&pcikcall)) {
		pcikcall.op = PCI_KCALL_OP_READ_CONFIG;
		pcikcall.ptr = cfg_space;
		kcall("pci", (unsigned int)&pcikcall);
		lspci_print_device(pcikcall.bus, pcikcall.device, pcikcall.function, cfg_space);
	}

	pcikcall.op = PCI_KCALL_OP_NEXT_ADDR;
	while (kcall("pci", (unsigned int)&pcikcall)) {
		pcikcall.op = PCI_KCALL_OP_READ_CONFIG;
		pcikcall.ptr = cfg_space;
		kcall("pci", (unsigned int)&pcikcall);
		lspci_print_device(pcikcall.bus, pcikcall.device, pcikcall.function, cfg_space);
		pcikcall.op = PCI_KCALL_OP_NEXT_ADDR;
	}
}
