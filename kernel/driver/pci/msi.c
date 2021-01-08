/*
 * PCI Message Signaled Interrupt
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

struct PCIMSIVector pci_msi_vector[PCI_MSI_VECTOR_MAX];

void pci_msi_intr(int vector) {
	int vec = vector - PCI_MSI_VECTOR_BASE;
	if (pci_msi_vector[vec].used) {
		pci_msi_vector[vec].handler(pci_msi_vector[vec].private);
	} else {
		cprintf("[pci] spurious MSI interrupt vector %d\n", vector);
	}
}

int pci_msi_alloc_vector(void (*handler)(void*), void* private) {
	for (int i = 0; i < PCI_MSI_VECTOR_MAX; i++) {
		if (!pci_msi_vector[i].used) {
			pci_msi_vector[i].used = 1;
			pci_msi_vector[i].handler = handler;
			pci_msi_vector[i].private = private;
			return i + PCI_MSI_VECTOR_BASE;
		}
	}
	return 0;
}

void pci_msi_free_vector(int vector) {
	pci_msi_vector[vector - PCI_MSI_VECTOR_BASE].used = 0;
}

static int pci_msi_probe(const struct PciAddress* addr, int* sixbits) {
	int capoff = pci_find_capability(addr, 5);
	if (!capoff) {
		return 0;
	}

	uint16_t msictl = pci_read_config_reg16(addr, capoff + 2);
	cprintf("[pci] MSI %d:%d.%d off %x 64bits%s Mask%s MulMsg %x\n", addr->bus, addr->device,
			addr->function, capoff, BOOL2SIGN(msictl & (1 << 7)), BOOL2SIGN(msictl & (1 << 8)),
			(msictl >> 1) & 7);

	*sixbits = msictl & (1 << 7);
	return capoff;
}

int pci_msi_enable(const struct PciAddress* addr, int vector, int lapicid) {
	int sixbits;
	int capoff = pci_msi_probe(addr, &sixbits);
	if (!capoff) {
		return 0;
	}

	pci_write_config_reg32(addr, capoff + 4, 0xfee00000 | lapicid << 12);
	if (sixbits) {
		pci_write_config_reg32(addr, capoff + 8, 0);
		pci_write_config_reg16(addr, capoff + 12, vector);
	} else {
		pci_write_config_reg16(addr, capoff + 8, vector);
	}
	uint16_t msictl = pci_read_config_reg16(addr, capoff + 2);
	msictl &= 0xff8e;
	msictl |= 1;
	pci_write_config_reg16(addr, capoff + 2, msictl);
	pci_enable_bus_mastering(addr);
	return capoff;
}

void pci_msi_disable(const struct PciAddress* addr) {
	int capoff = pci_find_capability(addr, 5);
	if (!capoff) {
		return;
	}
	uint16_t msictl = pci_read_config_reg16(addr, capoff + 2);
	msictl &= ~1;
	pci_write_config_reg16(addr, capoff + 2, msictl);
}
