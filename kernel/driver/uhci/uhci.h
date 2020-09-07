#ifndef _UHCI_H
#define _UHCI_H

struct UHCIDevice;
struct USBPacket;

void uhci_init(void);
void uhci_port_reset(struct UHCIDevice* uhci, int port);
enum USBStatus uhci_transfer(struct UHCIDevice* dev, unsigned int addr,
							 unsigned int endpoint, const struct USBPacket* packets,
							 int num);

#endif
