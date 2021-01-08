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
#include <driver/usb/usb.h>
#include <memlayout.h>

#include "uhci-regs.h"

struct UHCIDevice {
	ioport_t iobase;
	uint32_t* frame_list;
};

static void uhci_intr(struct PCIDevice* pcidev) {}

static void uhci_port_reset(void* private, unsigned int port) {
	struct UHCIDevice* uhci = private;
	uint16_t portstat = inw(uhci->iobase + UHCI_PORTSC + port * 2);
	portstat |= UHCI_PORTSC_PORT_RESET;
	outw(uhci->iobase + UHCI_PORTSC + port * 2, portstat);
	portstat &= ~UHCI_PORTSC_PORT_RESET;
	outw(uhci->iobase + UHCI_PORTSC + port * 2, portstat);
	portstat |= UHCI_PORTSC_PORT_ENABLE;
	outw(uhci->iobase + UHCI_PORTSC + port * 2, portstat);
}

static enum USBPortStatus uhci_get_port_status(void* private, unsigned int port) {
	struct UHCIDevice* dev = private;
	uint16_t portstat = inw(dev->iobase + UHCI_PORTSC + port * 2);
	if (portstat & UHCI_PORTSC_CONNECT_STATUS) {
		if (portstat & UHCI_PORTSC_LOW_SPEED) {
			return USB_PORT_STATUS_CONNECT_LOW_SPEED;
		} else {
			return USB_PORT_STATUS_CONNECT_FULL_SPEED;
		}
	}
	return USB_PORT_STATUS_NOT_CONNECT;
}

static void uhci_run(struct UHCIDevice* dev, int frnum) {
	outw(dev->iobase + UHCI_FRNUM, frnum);
	outw(dev->iobase + UHCI_USBCMD, inw(dev->iobase + UHCI_USBCMD) | UHCI_USBCMD_RS);
}

static void uhci_stop(struct UHCIDevice* dev) {
	outw(dev->iobase + UHCI_USBSTS, 1);
	outw(dev->iobase + UHCI_USBCMD, inw(dev->iobase + UHCI_USBCMD) & ~UHCI_USBCMD_RS);
}

static enum USBTransferStatus uhci_transfer_packet(void* private, unsigned int addr,
												   unsigned int endpoint,
												   const struct USBPacket* packets,
												   unsigned int num) {
	struct UHCIDevice* dev = private;
	for (int i = 0; i < 1024; i++) {
		dev->frame_list[i] = 1;
	}
	volatile struct UHCITransferDesc {
		uint32_t lnk, sta, maxlen, bufptr;
	} PACKED* transdesc = kalloc();
	for (unsigned int i = 0; i < num; i++) {
		transdesc[i].lnk = 1;
		transdesc[i].sta = UHCI_STATUS_ACTIVE;
		if (i == num - 1) {
			transdesc[i].sta |= (1 << 24);
		}
		if (inw(dev->iobase + UHCI_PORTSC) & UHCI_PORTSC_LOW_SPEED) {
			transdesc[i].sta |= UHCI_STATUS_LOWSPEED;
		}
		transdesc[i].maxlen = ((packets[i].maxlen - 1) << 21) | (endpoint << 15) | (addr << 8);
		if (packets[i].type == USB_PACKET_IN) {
			transdesc[i].maxlen |= UHCI_TOKEN_PID_IN;
		} else if (packets[i].type == USB_PACKET_OUT) {
			transdesc[i].maxlen |= UHCI_TOKEN_PID_OUT;
		} else if (packets[i].type == USB_PACKET_SETUP) {
			transdesc[i].maxlen |= UHCI_TOKEN_PID_SETUP;
		}
		if (packets[i].toggle) {
			transdesc[i].maxlen |= UHCI_TOKEN_TOGGLE;
		}
		transdesc[i].bufptr = V2P(packets[i].buffer);
		dev->frame_list[i] = V2P(transdesc + i);
	}

	uhci_run(dev, 0);

	while (!(inw(dev->iobase + UHCI_USBSTS) & 1)) {
	}
	uhci_stop(dev);
	kfree((void*)transdesc);
	return USB_STATUS_OK;
}

const static struct USBHostControllerDriver uhci_usbhc_driver = {
	.reset_port = uhci_port_reset,
	.get_port_status = uhci_get_port_status,
	.transfer_packet = uhci_transfer_packet,
};

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
	usb_register_host_controller(dev, "uhci", 2, &uhci_usbhc_driver);
}

struct PCIDriver uhci_pci_driver = {
	.name = "uhci",
	.class_type = 0x0c0300,
	.init = uhci_controller_init,
};

void uhci_init(void) {
	pci_register_driver(&uhci_pci_driver);
}
