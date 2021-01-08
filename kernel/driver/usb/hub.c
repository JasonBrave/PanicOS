/*
 * USB Hub driver
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

#include "hub-defs.h"
#include "usb-struct.h"
#include "usb.h"

struct USBHub {
	struct USBInterface* usbif;
	unsigned int num_ports;
};

static void usb_hub_reset_port(struct USBHub* hub, unsigned int port) {
	struct USBSetupData* setup = kalloc();
	setup->bm_request_type =
		USB_REQUEST_DIR_HOST_TO_DEVICE | USB_REQUEST_TYPE_CLASS | USB_REQUEST_RECP_OTHER;
	setup->b_request = USB_HUB_REQUEST_SET_FEATURE;
	setup->w_value = USB_HUB_FEATURE_PORT_RESET;
	setup->w_index = port + 1;
	setup->w_length = 0;
	usb_control_transfer_nodata(hub->usbif->usbdev->bus, hub->usbif->usbdev->addr, 0, setup);
	kfree(setup);
}

static void usb_hub_dev_init(struct USBInterface* usbif) {
	volatile struct USBHubDescriptor* hub_desc = kalloc();
	if (usb_get_class_descriptor(usbif->usbdev, 0x29, 0, (void*)hub_desc,
								 sizeof(struct USBHubDescriptor))) {
		cprintf("[usb-hub] Hub GetHubDescriptor failed\n");
		return;
	}
	struct USBHub* hub = kalloc();
	memset(hub, 0, sizeof(struct USBHub));
	hub->usbif = usbif;
	usbif->private = hub;
	hub->num_ports = hub_desc->num_ports;
	cprintf("[usb-hub] USB Hub num_ports %d\n", hub->num_ports);

	for (unsigned int port = 0; port < hub->num_ports; port++) {
		struct USBSetupData* setup = kalloc();
		volatile uint16_t* port_status_word = kalloc();
		setup->bm_request_type =
			USB_REQUEST_DIR_DEVICE_TO_HOST | USB_REQUEST_TYPE_CLASS | USB_REQUEST_RECP_OTHER;
		setup->b_request = USB_HUB_REQUEST_GET_STATUS;
		setup->w_value = 0;
		setup->w_index = port + 1;
		setup->w_length = 4;
		usb_control_transfer_in(usbif->usbdev->bus, usbif->usbdev->addr, 0, setup,
								(void*)port_status_word, 4);
		kfree(setup);
		if (port_status_word[0] & USB_HUB_PORT_STATUS_CONNECTION) {
			if (port_status_word[0] & USB_HUB_PORT_STATUS_LOW_SPEED) {
				cprintf("[usb-hub] Low-speed device on port %d\n", port);
			} else {
				cprintf("[usb-hub] Full-speed device on port %d\n", port);
			}
			usb_hub_reset_port(hub, port);
			usb_register_device(usbif->usbdev->bus, port);
		}
		kfree((void*)port_status_word);
	}
}

static void usb_hub_dev_uninit(struct USBInterface* usbif) {}

const static struct USBDriver usb_hub_driver = {
	.name = "usb-hub",
	.class_type = 0x090000,
	.init = usb_hub_dev_init,
	.uninit = usb_hub_dev_uninit,
};

void usb_hub_init(void) {
	usb_register_driver(&usb_hub_driver);
}
