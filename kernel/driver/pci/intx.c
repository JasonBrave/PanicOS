/*
 * PCI INTx interrupt support
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

#include "pci-config.h"
#include "pci.h"

struct PciIrqInfo pci_irq_10[PCI_IRQ_MAX], pci_irq_11[PCI_IRQ_MAX];

int pci_irq_10_top = 0, pci_irq_11_top = 0;

void pci_add_irq(int irq, const struct PciAddress* addr) {
	if (irq == 10) {
		pci_irq_10[pci_irq_10_top].addr = *addr;
		pci_irq_10_top++;
	} else if (irq == 11) {
		pci_irq_11[pci_irq_11_top].addr = *addr;
		pci_irq_11_top++;
	}
}

void pci_interrupt(int irq) {
	if (irq == 10) {
		for (int i = 0; i < pci_irq_10_top; i++) {
			if (pci_read_config_reg8(&pci_irq_10[i].addr, PCI_CONF_STATUS) &
				PCI_STATUS_INTERRUPT) {
				struct PciAddress* addr = &pci_irq_10[i].addr;
				void (*handler)(const struct PciAddress*) = pci_irq_10[i].handler;
				if (handler) {
					handler(addr);
				}
			}
		}
	} else if (irq == 11) {
		for (int i = 0; i < pci_irq_11_top; i++) {
			if (pci_read_config_reg8(&pci_irq_11[i].addr, PCI_CONF_STATUS) &
				PCI_STATUS_INTERRUPT) {
				struct PciAddress* addr = &pci_irq_11[i].addr;
				void (*handler)(const struct PciAddress*) = pci_irq_11[i].handler;
				if (handler) {
					handler(addr);
				}
			}
		}
	}
}

void pci_register_intr_handler(const struct PciAddress* addr,
							   void (*handler)(const struct PciAddress*)) {
	struct PciAddress* intraddr;
	for (int i = 0; i < pci_irq_10_top; i++) {
		intraddr = &pci_irq_10[i].addr;
		if ((intraddr->bus == addr->bus) && (intraddr->device == addr->device) &&
			(intraddr->function == addr->function)) {
			pci_irq_10[i].handler = handler;
		}
	}
	for (int i = 0; i < pci_irq_11_top; i++) {
		intraddr = &pci_irq_11[i].addr;
		if ((intraddr->bus == addr->bus) && (intraddr->device == addr->device) &&
			(intraddr->function == addr->function)) {
			pci_irq_11[i].handler = handler;
		}
	}
}
