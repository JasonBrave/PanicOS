#ifndef _USB_STRUCT_H
#define _USB_STRUCT_H

#include <common/types.h>

#define USB_REQUEST_DIR_HOST_TO_DEVICE (0 << 7)
#define USB_REQUEST_DIR_DEVICE_TO_HOST (1 << 7)
#define USB_REQUEST_TYPE_STANDARD (0 << 5)
#define USB_REQUEST_TYPE_CLASS (1 << 5)
#define USB_REQUEST_TYPE_VENDOR (2 << 5)
#define USB_REQUEST_RECP_DEVICE (0 << 0)
#define USB_REQUEST_RECP_INTERFACE (1 << 0)
#define USB_REQUEST_RECP_ENDPOINT (2 << 0)
#define USB_REQUEST_RECP_OTHER (3 << 0)

enum USBStandardRequest {
	USB_REQUEST_GET_STATUS = 0,
	USB_REQUEST_CLEAR_FEATURE = 1,
	USB_REQUEST_SET_FEATURE = 3,
	USB_REQUEST_SET_ADDRESS = 5,
	USB_REQUEST_GET_DESCRIPTOR = 6,
	USB_REQUEST_SET_DESCRIPTOR = 7,
	USB_REQUEST_GET_CONFIGURATION = 8,
	USB_REQUEST_SET_CONFIGURATION = 9,
	USB_REQUEST_GET_INTERFACE = 10,
	USB_REQUEST_SET_INTERFACE = 11,
	USB_REQUEST_SYNCH_FRAME = 12,
};

enum USBDescriptorTypes {
	USB_DESCRIPTOR_DEVICE = 1,
	USB_DESCRIPTOR_CONFIGURATION = 2,
	USB_DESCRIPTOR_STRING = 3,
	USB_DESCRIPTOR_INTERFACE = 4,
	USB_DESCRIPTOR_ENDPOINT = 5,
};

struct USBSetupData {
	uint8_t bm_request_type;
	uint8_t b_request;
	uint16_t w_value;
	uint16_t w_index;
	uint16_t w_length;
} PACKED;

struct USBDeviceDescriptor {
	uint8_t length;
	uint8_t descriptor_type;
	uint16_t bcdusb;
	uint8_t class;
	uint8_t subclass;
	uint8_t protocol;
	uint8_t max_packet_size;
	uint16_t vendor;
	uint16_t product;
	uint16_t bcddevice;
	uint8_t manufacturer_index;
	uint8_t product_index;
	uint8_t serial_number_index;
	uint8_t num_configurations;
} PACKED;

struct USBConfigurationDescriptor {
	uint8_t length;
	uint8_t descriptor_type;
	uint16_t total_length;
	uint8_t num_interfaces;
	uint8_t configuration_value;
	uint8_t configuration_string;
	uint8_t attributes;
	uint8_t max_power;
} PACKED;

struct USBInterfaceDescriptor {
	uint8_t length;
	uint8_t descriptor_type;
	uint8_t interface_number;
	uint8_t alternate_setting;
	uint8_t num_endpoints;
	uint8_t interface_class;
	uint8_t interface_subclass;
	uint8_t interface_protocol;
	uint8_t interface_string;
} PACKED;

struct USBEndpointDescriptor {
	uint8_t length;
	uint8_t descriptor_type;
	uint8_t endpoint_address;
	uint8_t attributes;
	uint16_t max_packet_size;
	uint8_t interval;
} PACKED;

#endif
