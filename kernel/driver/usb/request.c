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

int usb_get_standard_descriptor(struct USBDevice* dev, unsigned int desc_type,
								unsigned int desc_index, void* buffer, unsigned int size) {
	struct USBSetupData* setup = kalloc();
	setup->bm_request_type = USB_REQUEST_DIR_DEVICE_TO_HOST | USB_REQUEST_TYPE_STANDARD;
	setup->b_request = USB_REQUEST_GET_DESCRIPTOR;
	setup->w_value = (desc_type << 8) | desc_index;
	setup->w_index = 0;
	setup->w_length = size;

	int status = usb_control_transfer_in(dev->bus, dev->addr, 0, setup, buffer, size);
	kfree(setup);
	return status;
}

int usb_get_class_descriptor(struct USBDevice* dev, unsigned int desc_type, unsigned int desc_index,
							 void* buffer, unsigned int size) {
	struct USBSetupData* setup = kalloc();
	setup->bm_request_type = USB_REQUEST_DIR_DEVICE_TO_HOST | USB_REQUEST_TYPE_CLASS;
	setup->b_request = USB_REQUEST_GET_DESCRIPTOR;
	setup->w_value = (desc_type << 8) | desc_index;
	setup->w_index = 0;
	setup->w_length = size;

	int status = usb_control_transfer_in(dev->bus, dev->addr, 0, setup, buffer, size);
	kfree(setup);
	return status;
}

int usb_get_device_descriptor(struct USBDevice* dev) {
	volatile struct USBDeviceDescriptor* device_desc = kalloc();

	int status = usb_get_standard_descriptor(dev, USB_DESCRIPTOR_DEVICE, 0, (void*)device_desc,
											 sizeof(struct USBDeviceDescriptor));

	if (status) {
		kfree((void*)device_desc);
		return -1;
	}
	dev->vendor_id = device_desc->vendor;
	dev->product_id = device_desc->product;
	dev->num_configuration = device_desc->num_configurations;
	dev->class = device_desc->class;
	dev->subclass = device_desc->subclass;
	dev->protocol = device_desc->protocol;
	kfree((void*)device_desc);
	return 0;
}

int usb_get_configuration_descriptor(struct USBDevice* dev, unsigned int index,
									 uint8_t* configuration_value) {
	volatile void* descriptor = kalloc();
	int status = usb_get_standard_descriptor(dev, USB_DESCRIPTOR_CONFIGURATION, index,
											 (void*)descriptor, 1024);

	if (status) {
		kfree((void*)descriptor);
		return -1;
	}

	struct USBConfigurationDescriptor* config_desc = (void*)descriptor;
	*configuration_value = config_desc->configuration_value;
	dev->num_interface = config_desc->num_interfaces;

	void* descptr = (void*)descriptor + sizeof(struct USBConfigurationDescriptor);
	int infcnt = 0;
	for (; descptr < descriptor + config_desc->total_length;) {
		struct USBGenericDescriptor {
			uint8_t length;
			uint8_t descriptor_type;
		} PACKED* d = descptr;
		if (d->descriptor_type == USB_DESCRIPTOR_INTERFACE) {
			struct USBInterfaceDescriptor* inf = descptr;
			dev->interfaces[infcnt].usbdev = dev;
			dev->interfaces[infcnt].class = inf->interface_class;
			dev->interfaces[infcnt].subclass = inf->interface_subclass;
			dev->interfaces[infcnt].protocol = inf->interface_protocol;
			dev->interfaces[infcnt].num_endpoints = inf->num_endpoints;
			infcnt++;
		}
		descptr += d->length;
	}

	kfree((void*)descriptor);
	return 0;
}

int usb_set_address(struct USBBus* bus, unsigned int oldaddr, unsigned int newaddr) {
	struct USBSetupData* setup = kalloc();
	setup->bm_request_type = 0;
	setup->b_request = USB_REQUEST_SET_ADDRESS;
	setup->w_value = newaddr;
	setup->w_index = 0;
	setup->w_length = 0;
	int status = usb_control_transfer_nodata(bus, oldaddr, 0, setup);
	kfree(setup);
	return status;
}

int usb_set_configuration(struct USBDevice* dev, uint8_t configuration_value) {
	struct USBSetupData* setup = kalloc();
	setup->bm_request_type = 0;
	setup->b_request = USB_REQUEST_SET_CONFIGURATION;
	setup->w_value = configuration_value;
	setup->w_index = 0;
	setup->w_length = 0;
	int status = usb_control_transfer_nodata(dev->bus, dev->addr, 0, setup);
	kfree(setup);
	return status;
}
