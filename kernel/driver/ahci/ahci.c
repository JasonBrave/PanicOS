/*
 * Advanced Host Controller Interface (AHCI) driver
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
#include <driver/pci/pci.h>

#include "ahci.h"
#include "ahci_reg.h"

static struct AHCIController *ahci_controller_new(void) {
	struct AHCIController *dev = kalloc();
	memset(dev, 0, sizeof(struct AHCIController));
	return dev;
}

static inline uint32_t ahci_read_reg32(struct AHCIController *ahci, unsigned int offset) {
	return ahci->abar[offset >> 2];
}

static inline void ahci_write_reg32(struct AHCIController *ahci, unsigned int offset,
									uint32_t value) {
	ahci->abar[offset >> 2] = value;
}

void ahci_controller_init(struct PCIDevice *pci_dev) {
	const struct PciAddress *pci_addr = &pci_dev->addr;
	struct AHCIController *ahci = ahci_controller_new();
	pci_dev->private = ahci;
	ahci->abar = map_mmio_region(pci_read_bar(pci_addr, 5), pci_read_bar_size(pci_addr, 5));
	cprintf("[ahci] AHCI Controller at PCI %x:%x.%x ABAR %x\n", pci_addr->bus, pci_addr->device,
			pci_addr->function, pci_read_bar(&pci_dev->addr, 5));
	// reset AHCI HBA
	ahci_write_reg32(ahci, AHCI_GHC, ahci_read_reg32(ahci, AHCI_GHC) | AHCI_GHC_HBARESET);
	while (ahci_read_reg32(ahci, AHCI_GHC) & AHCI_GHC_HBARESET) {
	}
	// set AHCI enable bit
	ahci_write_reg32(ahci, AHCI_GHC, ahci_read_reg32(ahci, AHCI_GHC) | AHCI_GHC_AHCIEN);
	// print AHCI version
	uint32_t ahciver = ahci_read_reg32(ahci, AHCI_VERSION);
	cprintf("[ahci] AHCI version %x.%x\n",
			(ahciver >> AHCI_VERSION_MAJOR_SHIFT) & AHCI_VERSION_MAJOR_MASK,
			(ahciver >> AHCI_VERSION_MINOR_SHIFT) & AHCI_VERSION_MINOR_MASK);
	// print AHCI capability
	uint32_t ahcicap = ahci_read_reg32(ahci, AHCI_CAP);
	cprintf("[ahci] AHCI 64Addr%s NCQ%s Speed %x AHCIOnly%s NCmdSlot %d NPorts %d\n",
			BOOL2SIGN(ahcicap & AHCI_CAP_S64A), BOOL2SIGN(ahcicap & AHCI_CAP_SNCQ),
			(ahcicap >> AHCI_CAP_ISS_SHIFT) & AHCI_CAP_ISS_MASK, BOOL2SIGN(ahcicap & AHCI_CAP_SAM),
			((ahcicap >> AHCI_CAP_NCMDS_SHIFT) & AHCI_CAP_NCMDS_MASK) + 1,
			((ahcicap >> AHCI_CAP_NPORTS_SHIFT) & AHCI_CAP_NPORTS_MASK) + 1);
	// AHCI BIOS OS handoff
	if (ahci_read_reg32(ahci, AHCI_CAP2) & AHCI_CAP2_BOH) {
		cprintf("[ahci] Start BIOS OS handoff");
	}
}

struct PCIDriver ahci_driver = {
	.name = "ahci",
	.class_type = 0x010601, // AHCI
	.init = ahci_controller_init,
};

void ahci_init(void) {
	pci_register_driver(&ahci_driver);
}
