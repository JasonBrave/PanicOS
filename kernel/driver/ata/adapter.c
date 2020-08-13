/*
 * PCI IDE Controller driver
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
#include <driver/pci/pci-config.h>
#include <driver/pci/pci.h>

#include "ata.h"

struct ATAAdapter ata_adapter[ATA_ADAPTER_MAX];

const static char* bus_master_str[] = {"Non-Bus Mastering", "Bus Mastering"};
const static char* pci_native_str[] = {"ISA Compatibility", "PCI Native"};

static struct ATAAdapter* ata_adapter_alloc(void) {
	for (int i = 0; i < ATA_ADAPTER_MAX; i++) {
		if (!ata_adapter[i].cmdblock_base[0]) {
			return &ata_adapter[i];
		}
	}
	return 0;
}

void ata_adapter_dev_init(const struct PciAddress* addr) {
	struct ATAAdapter* adapter = ata_adapter_alloc();
	if (!adapter) {
		panic("too many ATA adapter");
	}

	const uint8_t progif = pci_read_config_reg8(addr, PCI_CONF_PROGIF);
	if (progif & (1 << 7)) {
		adapter->bus_master = 1;
	}
	if ((progif & (1 << 0)) && (progif & (1 << 2))) {
		adapter->pci_native = 1;
	}
	cprintf("[ata] Adapter %d:%d.%d %s %s IDE Controller\n", addr->bus, addr->device,
			addr->function, bus_master_str[adapter->bus_master],
			pci_native_str[adapter->pci_native]);

	// read base addresses
	if (adapter->pci_native) {
		adapter->cmdblock_base[0] = pci_read_bar(addr, 0);
		adapter->control_base[0] = pci_read_bar(addr, 1);
		adapter->cmdblock_base[1] = pci_read_bar(addr, 2);
		adapter->control_base[1] = pci_read_bar(addr, 3);
	} else {
		adapter->cmdblock_base[0] = 0x1f0;
		adapter->control_base[0] = 0x3f6;
		adapter->cmdblock_base[1] = 0x170;
		adapter->control_base[1] = 0x376;
	}
	if (adapter->bus_master) {
		adapter->bus_master_base = pci_read_bar(addr, 4);
	}
	cprintf("[ata] cmd0 0x%x ctl0 0x%x cmd1 0x%x ctl1 0x%x bmdma 0x%x\n",
			adapter->cmdblock_base[0], adapter->control_base[0],
			adapter->cmdblock_base[1], adapter->control_base[1],
			adapter->bus_master_base);
	// enable bus mastering
	if (adapter->bus_master) {
		uint16_t pcicmd = pci_read_config_reg16(addr, PCI_CONF_COMMAND);
		pcicmd |= PCI_CONTROL_BUS_MASTER;
		pci_write_config_reg16(addr, PCI_CONF_COMMAND, pcicmd);
	}
	// enable PCI interrupt
	if (adapter->pci_native) {
		pci_register_intr_handler(addr, ata_pci_intr);
	}
}

void ata_adapter_init(void) {
	memset(&ata_adapter, 0, sizeof(ata_adapter));
	// enable legacy IRQ
	ioapicenable(14, 0);
	ioapicenable(15, 0);

	struct PciAddress addr;
	if (pci_find_class(&addr, 1, 1)) {
		ata_adapter_dev_init(&addr);
		while (pci_next_class(&addr, 1, 1)) {
			ata_adapter_dev_init(&addr);
		}
	}
}

void ata_legacy_intr(int irq) {
	cprintf("[ata] IRQ %d\n", irq);
}

void ata_pci_intr(const struct PciAddress* addr) {
	cprintf("[ata] PCI intr\n");
}
