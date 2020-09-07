#ifndef _USB_STRUCT_H
#define _USB_STRUCT_H

#include <common/types.h>

#define USB_REQUEST_TYPE_DEVICE_TO_HOST (1 << 7)

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

struct USBDevicedesc {
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

#endif
