/*
 * Kernel module USB helper functions
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

#ifndef _MODLIB_USB_H
#define _MODLIB_USB_H

#include <kernsrv.h>

enum USBTransferStatus {
	USB_STATUS_OK = 0,
	USB_STAUTS_STALL,
	USB_STAUTS_NAK,
	USB_STAUTS_CRC_ERROR,
	USB_STAUTS_BABBLE,
};

enum USBPortStatus {
	USB_PORT_STATUS_NOT_CONNECT,
	USB_PORT_STATUS_CONNECT_LOW_SPEED,
	USB_PORT_STATUS_CONNECT_FULL_SPEED,
};

struct USBInterface {
	const struct USBDriver* driver;
	struct USBDevice* usbdev;
	void* private;
	uint8_t class, subclass, protocol;
	unsigned int num_endpoints;
};

#define USB_INTERFACE_MAX_NUM 16

struct USBDevice {
	struct USBBus* bus;
	unsigned int addr, port;
	uint16_t vendor_id, product_id;
	unsigned int num_configuration, num_interface;
	uint8_t class, subclass, protocol;
	struct USBInterface interfaces[USB_INTERFACE_MAX_NUM];
};

struct USBDriver {
	const char* name;
	uint32_t class_type;
	void (*init)(struct USBInterface*);
	void (*uninit)(struct USBInterface*);
};

struct USBPacket {
	enum USBPacketType {
		USB_PACKET_IN,
		USB_PACKET_OUT,
		USB_PACKET_SETUP,
	} type;
	unsigned int maxlen;
	void* buffer;
	struct {
		unsigned char toggle : 1;
	};
};

struct USBHostControllerDriver {
	void (*reset_port)(void* private, unsigned int port);
	enum USBPortStatus (*get_port_status)(void* private, unsigned int port);
	enum USBTransferStatus (*transfer_packet)(void* private, unsigned int addr,
											  unsigned int endpoint,
											  const struct USBPacket* packets,
											  unsigned int num);
};

static inline void
usb_register_host_controller(void* private, const char* name, unsigned int num_ports,
							 const struct USBHostControllerDriver* hcdriver) {
	return kernsrv->usb_register_host_controller(private, name, num_ports, hcdriver);
}

static inline void usb_register_driver(const struct USBDriver* driver) {
	return kernsrv->usb_register_driver(driver);
}

static inline enum USBTransferStatus
usb_control_transfer_in(struct USBBus* bus, unsigned int addr, unsigned int endpoint,
						void* setup, void* payload, int size) {
	return kernsrv->usb_control_transfer_in(bus, addr, endpoint, setup, payload, size);
}

static inline enum USBTransferStatus usb_control_transfer_nodata(struct USBBus* bus,
																 unsigned int addr,
																 unsigned int endpoint,
																 void* setup) {
	return usb_control_transfer_nodata(bus, addr, endpoint, setup);
}

static inline int usb_get_standard_descriptor(struct USBDevice* dev,
											  unsigned int desc_type,
											  unsigned int desc_index, void* buffer,
											  unsigned int size) {
	return usb_get_standard_descriptor(dev, desc_type, desc_index, buffer, size);
}

static inline int usb_get_class_descriptor(struct USBDevice* dev,
										   unsigned int desc_type,
										   unsigned int desc_index, void* buffer,
										   unsigned int size) {
	return usb_get_class_descriptor(dev, desc_type, desc_index, buffer, size);
}

static inline int usb_get_device_descriptor(struct USBDevice* dev) {
	return usb_get_device_descriptor(dev);
}

static inline int usb_get_configuration_descriptor(struct USBDevice* dev,
												   unsigned int index,
												   uint8_t* configuration_value) {
	return usb_get_configuration_descriptor(dev, index, configuration_value);
}

static inline int usb_set_configuration(struct USBDevice* dev,
										uint8_t configuration_value) {
	return usb_set_configuration(dev, configuration_value);
}

#endif
