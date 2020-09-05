/*
 * Universal Host Controller Interface driver
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
#include <driver/pci/pci.h>
#include <memlayout.h>

#include "uhci-regs.h"

struct UHCIDevice {
	ioport_t iobase;
	uint32_t* frame_list;
};

static void uhci_intr(struct PCIDevice* pcidev) {}

static void uhci_controller_init(struct PCIDevice* pcidev) {
	pci_enable_bus_mastering(&pcidev->addr);
	pci_register_intr_handler(pcidev, uhci_intr);
	struct UHCIDevice* dev = kalloc();
	pcidev->private = dev;
	dev->iobase = pci_read_bar(&pcidev->addr, 4);
	cprintf("[uhci] UHCI Controller IOBASE %x release %x\n", dev->iobase,
			pci_read_config_reg8(&pcidev->addr, 0x60));

	outw(dev->iobase + UHCI_USBCMD, UHCI_USBCMD_HCRESET);
	outw(dev->iobase + UHCI_USBCMD, 0);

	outw(dev->iobase + UHCI_USBINTR, (1 << 2));

	outb(dev->iobase + UHCI_SOFMOD, 64);
	dev->frame_list = kalloc();
	for (int i = 0; i < 1024; i++) {
		dev->frame_list[i] = 1;
	}
	outdw(dev->iobase + UHCI_FRBASEADD, V2P(dev->frame_list));

	for (int i = 0; i < 2; i++) {
		uint16_t portstat = inw(dev->iobase + UHCI_PORTSC + i * 2);
		if (portstat & UHCI_PORTSC_CONNECT_STATUS) {
			if (portstat & UHCI_PORTSC_LOW_SPEED) {
				cprintf("[uhci] low speed device on port %d\n", i);
			} else {
				cprintf("[uhci] full speed device on port %d\n", i);
			}
		}
	}
}

struct PCIDriver uhci_pci_driver = {
	.name = "uhci",
	.class_type = 0x0c0300,
	.init = uhci_controller_init,
};

void uhci_init(void) {
	pci_register_driver(&uhci_pci_driver);
}
