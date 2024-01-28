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

struct USBBus usb_bus[USB_BUS_NUM_MAX];

static unsigned int usb_alloc_address(struct USBBus *bus) {
	for (int i = 1; i <= 127; i++) {
		if (!bus->devices[i]) {
			return i;
		}
	}
	return 0;
}

struct USBBus *usb_alloc_bus(void) {
	for (int i = 0; i < USB_BUS_NUM_MAX; i++) {
		if (!usb_bus[i].controller.driver) {
			return &usb_bus[i];
		}
	}
	panic("too many usb bus");
}

// device at address 0, already reset by caller
void usb_register_device(struct USBBus *bus, unsigned int port) {
	unsigned int addr = usb_alloc_address(bus);
	struct USBDevice *usbdev = kalloc();
	memset(usbdev, 0, sizeof(struct USBDevice));
	bus->devices[addr] = usbdev;

	usbdev->bus = bus;
	usbdev->port = port;
	if (usb_get_device_descriptor(usbdev) != 0) {
		cprintf("[usb] GetDeviceDescriptor on port %d failed\n", port);
	}
	usbdev->addr = addr;
	if (usb_set_address(bus, 0, addr) != 0) {
		cprintf("[usb] SetAddress on port %d failed\n", port);
	}

	if (usbdev->class) {
		cprintf("[usb] port %d addr %d %x:%x class %d subclass %d protocol %d "
				"numcfg %d\n",
				port, addr, usbdev->vendor_id, usbdev->product_id, usbdev->class, usbdev->subclass,
				usbdev->protocol, usbdev->num_configuration);
	} else {
		cprintf("[usb] port %d addr %d %x:%x numcfg %d\n", port, addr, usbdev->vendor_id,
				usbdev->product_id, usbdev->num_configuration);
	}

	uint8_t configuration_value;
	if (usb_get_configuration_descriptor(usbdev, 0, &configuration_value) != 0) {
		cprintf("[usb] GetConfigurationDescriptor on port %d failed\n", port);
	}
	for (unsigned int i = 0; i < usbdev->num_interface; i++) {
		cprintf("[usb] config %d interface %d class %x subclass %x protocol %x "
				"endpoints %d\n",
				configuration_value, i, usbdev->interfaces[i].class, usbdev->interfaces[i].subclass,
				usbdev->interfaces[i].protocol, usbdev->interfaces[i].num_endpoints);
	}
	usb_set_configuration(usbdev, configuration_value);
}

void usb_register_host_controller(void *private, const char *name, unsigned int num_ports,
								  const struct USBHostControllerDriver *hcdriver) {
	struct USBBus *bus = usb_alloc_bus();
	bus->controller.name = name;
	bus->controller.driver = hcdriver;
	bus->controller.private = private;
	bus->controller.num_ports = num_ports;

	for (unsigned int port = 0; port < num_ports; port++) {
		hcdriver->reset_port(private, port);

		if (hcdriver->get_port_status(private, port) == USB_PORT_STATUS_NOT_CONNECT) {
			continue;
		}

		usb_register_device(bus, port);
	}
}

void usb_register_driver(const struct USBDriver *driver) {
	for (unsigned int bus = 0; bus < USB_BUS_NUM_MAX; bus++) {
		if (!usb_bus[bus].controller.driver) {
			continue;
		}
		for (unsigned int i = 0; i < USB_DEVICE_MAX_NUM; i++) {
			if (!usb_bus[bus].devices[i]) {
				continue;
			}
			for (unsigned int j = 0; j < usb_bus[bus].devices[i]->num_interface; j++) {
				if (!usb_bus[bus].devices[i]->interfaces[j].driver
					&& usb_bus[bus].devices[i]->interfaces[j].class
						   == (driver->class_type >> 16 & 0xff)
					&& usb_bus[bus].devices[i]->interfaces[j].subclass
						   == (driver->class_type >> 8 & 0xff)
					&& usb_bus[bus].devices[i]->interfaces[j].protocol
						   == (driver->class_type & 0xff)) {
					usb_bus[bus].devices[i]->interfaces[j].driver = driver;
					driver->init(&usb_bus[bus].devices[i]->interfaces[j]);
				}
			}
		}
	}
}

void usb_init(void) {
	memset(usb_bus, 0, sizeof(usb_bus));
#ifndef __riscv
	uhci_init();
#endif
	// usb device types
	usb_hub_init(); // USB hub
}

void usb_print_devices(void) {
	for (int bus = 0; bus < USB_BUS_NUM_MAX; bus++) {
		if (!usb_bus[bus].controller.driver) {
			continue;
		}
		cprintf("USB devices on bus #%d:\n", bus);
		for (int i = 0; i <= 127; i++) {
			if (usb_bus[bus].devices[i] && usb_bus[bus].devices[i]->class) {
				cprintf("|- Addr %d %x:%x class %d subclass %d protocol %d\n", i,
						usb_bus[bus].devices[i]->vendor_id, usb_bus[bus].devices[i]->product_id,
						usb_bus[bus].devices[i]->class, usb_bus[bus].devices[i]->subclass,
						usb_bus[bus].devices[i]->protocol);
			} else if (usb_bus[bus].devices[i]) {
				cprintf("|- Addr %d %x:%x\n", i, usb_bus[bus].devices[i]->vendor_id,
						usb_bus[bus].devices[i]->product_id);
			}
			if (usb_bus[bus].devices[i]) {
				for (unsigned int j = 0; j < usb_bus[bus].devices[i]->num_interface; j++) {
					if (usb_bus[bus].devices[i]->interfaces[j].driver) {
						cprintf("|  |- Interface class %d subclass %d protocol %d "
								"driver %s\n",
								usb_bus[bus].devices[i]->interfaces[j].class,
								usb_bus[bus].devices[i]->interfaces[j].subclass,
								usb_bus[bus].devices[i]->interfaces[j].protocol,
								usb_bus[bus].devices[i]->interfaces[j].driver->name);
					} else {
						cprintf("|  |- Interface class %d subclass %d protocol %d "
								"driver <none>\n",
								usb_bus[bus].devices[i]->interfaces[j].class,
								usb_bus[bus].devices[i]->interfaces[j].subclass,
								usb_bus[bus].devices[i]->interfaces[j].protocol);
					}
				}
			}
		}
	}
}
