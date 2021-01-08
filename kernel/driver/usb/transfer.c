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

#include "usb-struct.h"
#include "usb.h"

enum USBTransferStatus usb_control_transfer_in(struct USBBus* bus, unsigned int addr,
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
	enum USBTransferStatus status = bus->controller.driver->transfer_packet(
		bus->controller.private, addr, endpoint, packets, pid);
	kfree(packets);
	return status;
}

enum USBTransferStatus usb_control_transfer_nodata(struct USBBus* bus, unsigned int addr,
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

	enum USBTransferStatus status = bus->controller.driver->transfer_packet(
		bus->controller.private, addr, endpoint, packets, 2);
	kfree(packets);
	return status;
}
