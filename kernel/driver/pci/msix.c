/*
 * PCI MSI-X support
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
#include <arch/x86/msi.h>
#include <defs.h>

#include "pci.h"

static unsigned int pci_msix_probe(const struct PciAddress* addr, unsigned int* table_bar,
								   unsigned int* table_offset, unsigned int* pba_bar,
								   unsigned int* pba_offset) {
	int capoff = pci_find_capability(addr, 0x11);
	if (!capoff) {
		return 0;
	}
	uint16_t msixctl = pci_read_config_reg16(addr, capoff + 2);
	uint32_t tablereg, pbareg;
	tablereg = pci_read_config_reg32(addr, capoff + 4);
	pbareg = pci_read_config_reg32(addr, capoff + 8);
	*table_bar = tablereg & 0x7;
	*table_offset = tablereg & 0xfffffff8;
	*pba_bar = pbareg & 0x7;
	*pba_offset = pbareg & 0xfffffff8;
	cprintf("[pci] MSI-X %d:%d.%d %d vectors Table BAR %d off %x PBA BAR %d PBA %x\n", addr->bus,
			addr->device, addr->function, (msixctl & 0x7ff) + 1, *table_bar, *table_offset,
			*pba_bar, *pba_offset);

	return capoff;
}

int pci_msix_enable(struct PCIDevice* pci_dev) {
	unsigned int capoff;
	unsigned int table_bar, table_offset, pba_bar, pba_offset;
	if (!(capoff =
			  pci_msix_probe(&pci_dev->addr, &table_bar, &table_offset, &pba_bar, &pba_offset))) {
		return 0;
	}
	// map MSI-X table and PBA
	void* table_va = map_mmio_region(pci_read_bar(&pci_dev->addr, table_bar),
									 pci_read_bar_size(&pci_dev->addr, table_bar));
	void* pba_va = map_mmio_region(pci_read_bar(&pci_dev->addr, pba_bar),
								   pci_read_bar_size(&pci_dev->addr, pba_bar));
	pci_dev->msix_table = table_va + table_offset;
	pci_dev->msix_pba = pba_va + pba_offset;
	// mask all interrupt vectors
	int num_vec = pci_msix_get_num_vectors(pci_dev);
	for (int i = 0; i < num_vec; i++) {
		pci_dev->msix_table[i * 4 + 3] = 1; // vector masked
	}
	// enable MSI-X in PCI cfg
	uint16_t msixctl = pci_read_config_reg16(&pci_dev->addr, capoff + 2);
	msixctl = (msixctl & 0x7ff) | (1 << 15);
	pci_write_config_reg16(&pci_dev->addr, capoff + 2, msixctl);
	return capoff;
}

unsigned int pci_msix_get_num_vectors(struct PCIDevice* pci_dev) {
	int capoff = pci_find_capability(&pci_dev->addr, 0x11);
	if (!capoff) {
		return 0;
	}
	uint16_t msixctl = pci_read_config_reg16(&pci_dev->addr, capoff + 2);
	return (msixctl & 0x7ff) + 1;
}

void pci_msix_set_message(struct PCIDevice* pci_dev, unsigned int vec,
						  const struct MSIMessage* msg) {
	pci_dev->msix_table[vec * 4 + 0] = msg->addr & 0xffffffff;
	pci_dev->msix_table[vec * 4 + 1] = (msg->addr >> 32) & 0xffffffff;
	pci_dev->msix_table[vec * 4 + 2] = msg->data;
}

void pci_msix_mask(struct PCIDevice* pci_dev, unsigned int vec) {
	pci_dev->msix_table[vec * 4 + 3] = 1;
}

void pci_msix_unmask(struct PCIDevice* pci_dev, unsigned int vec) {
	pci_dev->msix_table[vec * 4 + 3] = 0;
}
