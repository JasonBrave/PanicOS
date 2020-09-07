#ifndef _USB_H
#define _USB_H

struct UHCIDevice;

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

enum USBStatus {
	USB_STATUS_OK = 0,
	USB_STAUTS_STALL,
	USB_STAUTS_NAK,
	USB_STAUTS_CRC_ERROR,
	USB_STAUTS_BABBLE,
};

// usb.c
void usb_init(void);
void usb_register_port(struct UHCIDevice* hci, int port);
void usb_print_devices(void);

#endif
