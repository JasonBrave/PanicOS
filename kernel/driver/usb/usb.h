#ifndef _USB_H
#define _USB_H

#include <common/types.h>

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

struct USBPacket;

enum USBPortStatus {
	USB_PORT_STATUS_NOT_CONNECT,
	USB_PORT_STATUS_CONNECT_LOW_SPEED,
	USB_PORT_STATUS_CONNECT_FULL_SPEED,
};

struct USBHostControllerDriver {
	void (*reset_port)(void* private, unsigned int port);
	enum USBPortStatus (*get_port_status)(void* private, unsigned int port);
	enum USBTransferStatus (*transfer_packet)(void* private, unsigned int addr,
											  unsigned int endpoint,
											  const struct USBPacket* packets,
											  unsigned int num);
};

struct USBHostController {
	const char* name;
	const struct USBHostControllerDriver* driver;
	void* private;
	unsigned int num_ports;
};

#define USB_DEVICE_MAX_NUM 128

struct USBBus {
	struct USBHostController controller;
	struct USBDevice* devices[USB_DEVICE_MAX_NUM];
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

enum USBTransferStatus {
	USB_STATUS_OK = 0,
	USB_STAUTS_STALL,
	USB_STAUTS_NAK,
	USB_STAUTS_CRC_ERROR,
	USB_STAUTS_BABBLE,
};

// usb.c
#define USB_BUS_NUM_MAX 16
extern struct USBBus usb_bus[USB_BUS_NUM_MAX];

void usb_init(void);
void usb_register_device(struct USBBus* bus, unsigned int port);
void usb_register_host_controller(void* private, const char* name,
								  unsigned int num_ports,
								  const struct USBHostControllerDriver* hcdriver);
void usb_register_driver(const struct USBDriver* driver);
void usb_print_devices(void);

// transfer.c
enum USBTransferStatus usb_control_transfer_in(struct USBBus* bus, unsigned int addr,
											   unsigned int endpoint, void* setup,
											   void* payload, int size);
enum USBTransferStatus usb_control_transfer_nodata(struct USBBus* bus,
												   unsigned int addr,
												   unsigned int endpoint, void* setup);
// request.c
int usb_get_standard_descriptor(struct USBDevice* dev, unsigned int desc_type,
								unsigned int desc_index, void* buffer,
								unsigned int size);
int usb_get_class_descriptor(struct USBDevice* dev, unsigned int desc_type,
							 unsigned int desc_index, void* buffer, unsigned int size);
int usb_get_device_descriptor(struct USBDevice* dev);
int usb_get_configuration_descriptor(struct USBDevice* dev, unsigned int index,
									 uint8_t* configuration_value);
int usb_set_address(struct USBBus* bus, unsigned int oldaddr, unsigned int newaddr);
int usb_set_configuration(struct USBDevice* dev, uint8_t configuration_value);

// hub.c
void usb_hub_init(void);

#endif
