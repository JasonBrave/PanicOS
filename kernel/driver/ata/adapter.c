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

#include <common/x86.h>
#include <defs.h>
#include <driver/pci/pci-config.h>
#include <driver/pci/pci.h>
#include <driver/x86/ioapic.h>
#include <memlayout.h>

#include "ata.h"

const static char* bus_master_str[] = {"Non-Bus Mastering", "Bus Mastering"};
const static char* pci_native_str[] = {"ISA Compatibility", "PCI Native"};

struct PATABMDMAPRD {
	uint32_t base;
	uint16_t size;
	uint16_t eot;
} PACKED;

static struct PATAAdapter* pata_adapter_alloc(void) {
	struct PATAAdapter* dev = kalloc();
	memset(dev, 0, sizeof(struct PATAAdapter));
	return dev;
}

void pata_adapter_dev_init(struct PCIDevice* pcidev) {
	const struct PciAddress* addr = &pcidev->addr;
	struct PATAAdapter* adapter = pata_adapter_alloc();

	pcidev->private = adapter;
	initlock(&adapter->lock[0], "pata-adapter");
	initlock(&adapter->lock[1], "pata-adapter");
	acquire(&adapter->lock[0]);
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
		for (int i = 0; i < 2; i++) {
			adapter->prd[i] = kalloc();
			memset(adapter->prd[i], 0, 4096);
		}
		pci_enable_bus_mastering(&pcidev->addr);
	}
	cprintf("[ata] cmd0 0x%x ctl0 0x%x cmd1 0x%x ctl1 0x%x bmdma 0x%x\n", adapter->cmdblock_base[0],
			adapter->control_base[0], adapter->cmdblock_base[1], adapter->control_base[1],
			adapter->bus_master_base);
	// enable PCI interrupt
	if (adapter->pci_native) {
		pci_register_intr_handler(pcidev, pata_adapter_pci_intr);
	}
	release(&adapter->lock[0]);
	pata_register_adapter(adapter);
}

void pata_adapter_bmdma_init(struct PATAAdapter* dev, int channel, int drive) {
	ioport_t bmdma_base = dev->bus_master_base + channel * 8;
	uint8_t bmdma_status = inb(bmdma_base + 2);
	bmdma_status |= 1 << (5 + drive) | 6;
	outb(bmdma_base + 2, bmdma_status);
}

void pata_adapter_bmdma_prepare(struct PATAAdapter* dev, int channel, phyaddr_t addr,
								unsigned int size) {
	volatile struct PATABMDMAPRD* prd = dev->prd[channel];
	prd[0].base = addr;
	prd[0].size = size;
	prd[0].eot = 0x8000;
	ioport_t bmdma_base = dev->bus_master_base + channel * 8;
	outdw(bmdma_base + 4, V2P(prd));
	uint8_t bmdma_status = inb(bmdma_base + 2);
	bmdma_status |= 6;
	outb(bmdma_base + 2, bmdma_status);
}

void pata_adapter_bmdma_start_write(struct PATAAdapter* dev, int channel) {
	ioport_t bmdma_base = dev->bus_master_base + channel * 8;
	uint8_t bmdma_command = inb(bmdma_base + 0);
	bmdma_command |= 9;
	outb(bmdma_base + 0, bmdma_command);
}

void pata_adapter_bmdma_start_read(struct PATAAdapter* dev, int channel) {
	ioport_t bmdma_base = dev->bus_master_base + channel * 8;
	uint8_t bmdma_command = inb(bmdma_base + 0);
	bmdma_command &= ~8;
	bmdma_command |= 1;
	outb(bmdma_base + 0, bmdma_command);
}

int pata_adapter_bmdma_busy(struct PATAAdapter* dev, int channel) {
	ioport_t bmdma_base = dev->bus_master_base + channel * 8;
	return inb(bmdma_base + 2) & 1;
}

void pata_adapter_bmdma_stop(struct PATAAdapter* dev, int channel) {
	ioport_t bmdma_base = dev->bus_master_base + channel * 8;
	outb(bmdma_base + 0, 0);
	uint8_t bmdma_status = inb(bmdma_base + 2);
	if (bmdma_status & 2) {
		cprintf("[ata] BMDMA error\n");
	}
	bmdma_status |= 6;
	outb(bmdma_base + 2, bmdma_status);
}

struct PCIDriver pata_adapter_pci_driver = {
	.name = "pci-ide-adapter",
	.class_type = 0x0101ff, // PCI IDE Controller
	.init = pata_adapter_dev_init,
};

void pata_adapter_init(void) {
	// enable legacy IRQ
	ioapic_enable(14, 0, IOAPIC_EDGE_TRIGGER, IOAPIC_ACTIVE_HIGH);
	ioapic_enable(15, 0, IOAPIC_EDGE_TRIGGER, IOAPIC_ACTIVE_HIGH);

	pci_register_driver(&pata_adapter_pci_driver);
}

void pata_adapter_legacy_intr(int irq) {}

void pata_adapter_pci_intr(struct PCIDevice* dev) {}
