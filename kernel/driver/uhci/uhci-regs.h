#ifndef _UHCI_REGS_H
#define _UHCI_REGS_H

#define UHCI_USBCMD 0x00
#define UHCI_USBCMD_MAXP (1 << 7)
#define UHCI_USBCMD_CF (1 << 6)
#define UHCI_USBCMD_SWDBG (1 << 5)
#define UHCI_USBCMD_FGR (1 << 4)
#define UHCI_USBCMD_EGSM (1 << 3)
#define UHCI_USBCMD_GRESET (1 << 2)
#define UHCI_USBCMD_HCRESET (1 << 1)
#define UHCI_USBCMD_RS (1 << 0)

#define UHCI_USBSTS 0x02

#define UHCI_USBINTR 0x04

#define UHCI_FRNUM 0x06

#define UHCI_FRBASEADD 0x08

#define UHCI_SOFMOD 0x0c

#define UHCI_PORTSC 0x10
#define UHCI_PORTSC_SUSPEND (1 << 12)
#define UHCI_PORTSC_PORT_RESET (1 << 9)
#define UHCI_PORTSC_LOW_SPEED (1 << 8)
#define UHCI_PORTSC_RESUME_DETECT (1 << 6)
#define UHCI_PORTSC_PORT_ENABLE_CHANGED (1 << 3)
#define UHCI_PORTSC_PORT_ENABLE (1 << 2)
#define UHCI_PORTSC_CONNECT_STATUS_CHANGED (1 << 1)
#define UHCI_PORTSC_CONNECT_STATUS (1 << 0)

#define UHCI_STATUS_ACTIVE (1 << 23)
#define UHCI_STATUS_LOWSPEED (1 << 26)
#define UHCI_TOKEN_TOGGLE (1 << 19)
#define UHCI_TOKEN_PID_IN 0x69
#define UHCI_TOKEN_PID_OUT 0xe1
#define UHCI_TOKEN_PID_SETUP 0x2d

#endif
