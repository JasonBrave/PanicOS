/*
 * Universal Serial Bus driver
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
#include <driver/uhci/uhci.h>

#include "usb-struct.h"
#include "usb.h"

struct USBDevice {
	uint16_t vendor_id, product_id;
	unsigned int num_configuration, num_interface;
	unsigned int port; // port on controller/hub
};

struct USBBus {
	struct UHCIDevice* controller;
	struct USBDevice* devices[128];
} usb_bus;

int usb_control_transfer_in(struct UHCIDevice* hci, unsigned int addr,
							unsigned int endpoint, void* setup, void* payload,
							int size) {
	struct USBPacket* packets = kalloc();
	memset(packets, 0, sizeof(struct USBPacket) * (2 + size / 8));

	packets[0].type = USB_PACKET_SETUP;
	packets[0].maxlen = 8;
	packets[0].buffer = setup;

	int toggle = 1;
	int pid = 1;
	for (int i = 0; i < size; i += 8) {
		packets[pid].type = USB_PACKET_IN;
		packets[pid].maxlen = 8;
		packets[pid].buffer = payload + i;
		packets[pid].toggle = toggle;
		toggle = toggle ? 0 : 1;
		pid++;
	}

	packets[pid].type = USB_PACKET_OUT;
	packets[pid].maxlen = 0x800;
	packets[pid].buffer = (void*)0x80000000;
	packets[pid].toggle = 1;
	int status = uhci_transfer(hci, addr, endpoint, packets, pid);
	kfree(packets);
	return status;
}

int usb_control_transfer_nodata(struct UHCIDevice* hci, unsigned int addr,
								unsigned int endpoint, void* setup) {
	struct USBPacket* packets = kalloc();
	memset(packets, 0, sizeof(struct USBPacket) * 2);

	packets[0].type = USB_PACKET_SETUP;
	packets[0].maxlen = 8;
	packets[0].buffer = setup;

	packets[1].type = USB_PACKET_IN;
	packets[1].maxlen = 0x800;
	packets[1].buffer = (void*)0x80000000;
	packets[1].toggle = 1;

	int status = uhci_transfer(hci, addr, endpoint, packets, 2);
	kfree(packets);
	return status;
}

static unsigned int usb_alloc_address(void) {
	for (int i = 1; i <= 127; i++) {
		if (!usb_bus.devices[i]) {
			return i;
		}
	}
	return 0;
}

int usb_get_device_descriptor(struct USBDevice* dev, unsigned int addr) {
	struct USBSetupData* setup = kalloc();
	setup->bm_request_type = (1 << 7);
	setup->b_request = USB_REQUEST_GET_DESCRIPTOR;
	setup->w_value = (USB_DESCRIPTOR_DEVICE << 8) | 0;
	setup->w_index = 0;
	setup->w_length = 18;

	volatile struct USBDevicedesc* device_desc = kalloc();

	int status = usb_control_transfer_in(usb_bus.controller, addr, 0, setup,
										 (void*)device_desc, 18);
	if (status) {
		kfree(setup);
		kfree((void*)device_desc);
		return -1;
	}
	dev->vendor_id = device_desc->vendor;
	dev->product_id = device_desc->product;
	dev->num_configuration = device_desc->num_configurations;
	kfree(setup);
	kfree((void*)device_desc);
	return 0;
}

int usb_set_address(unsigned int oldaddr, unsigned int newaddr) {
	struct USBSetupData* setup = kalloc();
	setup->bm_request_type = 0;
	setup->b_request = USB_REQUEST_SET_ADDRESS;
	setup->w_value = newaddr;
	setup->w_index = 0;
	setup->w_length = 0;
	int status = usb_control_transfer_nodata(usb_bus.controller, oldaddr, 0, setup);
	kfree(setup);
	return status;
}

void usb_register_port(struct UHCIDevice* hci, int port) {
	if (!usb_bus.controller) {
		usb_bus.controller = hci;
	}
	if (usb_bus.controller != hci) {
		return;
	}

	uhci_port_reset(hci, port);

	unsigned int addr = usb_alloc_address();
	usb_bus.devices[addr] = kalloc();

	struct USBDevice* usbdev = usb_bus.devices[addr];
	usbdev->port = port;
	if (usb_get_device_descriptor(usbdev, 0) != 0) {
		cprintf("[usb] GetDeviceDescriptor on port %d failed\n", port);
	}
	if (usb_set_address(0, addr) != 0) {
		cprintf("[usb] SetAddress on port %d failed\n", port);
	}

	cprintf("[usb] port %d addr %d %x:%x numcfg %d\n", port, addr, usbdev->vendor_id,
			usbdev->product_id, usbdev->num_configuration);
}

void usb_init(void) {
	memset(&usb_bus, 0, sizeof(usb_bus));
	uhci_init();
}

void usb_print_devices(void) {
	if (!usb_bus.controller) {
		cprintf("No USB controller found\n");
		return;
	}
	cprintf("USB devices on controller #1:\n");
	for (int i = 0; i <= 127; i++) {
		if (usb_bus.devices[i]) {
			cprintf("Port %d addr %d %x:%x\n", usb_bus.devices[i]->port, i,
					usb_bus.devices[i]->vendor_id, usb_bus.devices[i]->product_id);
		}
	}
}
