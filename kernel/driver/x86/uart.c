/*
 * 16550A UART driver
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

#include <common/x86.h>
#include <defs.h>
#include <driver/x86/ioapic.h>

#define NUM_UART 4

struct UARTDevice {
	ioport_t iobase;
	unsigned int exist;
} uart_devices[NUM_UART] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};

#define UART_REG_CHAR 0
#define UART_REG_INTR_ENABLE 1
#define UART_REG_INTR_ENABLE_DATA_READY (1 << 0)
#define UART_REG_FIFO 2
#define UART_REG_LINE_CONTROL 3
#define UART_REG_LINE_CONTROL_DLAB (1 << 7)
#define UART_REG_MODEM_CONTROL 4
#define UART_REG_LINE_STATUS 5
#define UART_REG_LINE_STATUS_DATA_READY (1 << 0)
#define UART_REG_LINE_STATUS_THR_EMPTY (1 << 5)
// when DLAB=1
#define UART_REG_DIVISOR_LO 0
#define UART_REG_DIVISOR_HI 1

static void uart_dev_init(struct UARTDevice* dev, ioport_t iobase, int irq) {
	dev->iobase = iobase;

	// If status is 0xFF, no serial port.
	if (inb(iobase + UART_REG_LINE_STATUS) == 0xFF) {
		return;
	} else {
		dev->exist = 1;
		cprintf("[uart] UART ioport %x irq %d baud 9600\n", iobase, irq);
	}

	// Turn off the FIFO
	outb(iobase + UART_REG_FIFO, 0);

	// 9600 baud, 8 data bits, 1 stop bit, parity off.
	outb(iobase + UART_REG_LINE_CONTROL, UART_REG_LINE_CONTROL_DLAB);
	outb(iobase + UART_REG_DIVISOR_LO, 115200 / 9600); // baud rate 9600
	outb(iobase + UART_REG_DIVISOR_HI, 0);
	outb(iobase + UART_REG_LINE_CONTROL, 0x03); // 8 data bits.
	outb(iobase + UART_REG_MODEM_CONTROL, 0);
	outb(iobase + UART_REG_INTR_ENABLE,
		 UART_REG_INTR_ENABLE_DATA_READY); // Enable receive interrupts.

	// Acknowledge pre-existing interrupt conditions;
	inb(iobase + UART_REG_FIFO);
	inb(iobase + UART_REG_CHAR);
}

void uart_init(void) {
	ioport_t iobases[] = {0x3f8, 0x2f8, 0x3e8, 0x2e8};
	int irqs[] = {4, 3, 4, 3};
	memset(uart_devices, 0, sizeof(uart_devices));
	for (int i = 0; i < NUM_UART; i++) {
		uart_dev_init(&uart_devices[i], iobases[i], irqs[i]);
	}
	ioapic_enable(3, 0, IOAPIC_EDGE_TRIGGER, IOAPIC_ACTIVE_HIGH);
	ioapic_enable(4, 0, IOAPIC_EDGE_TRIGGER, IOAPIC_ACTIVE_HIGH);
}

void uart_putc(unsigned char c) {
	if (!uart_devices[0].exist)
		return;
	ioport_t iobase = uart_devices[0].iobase;
	/*for (int i = 0;
		 i < 128 && !(inb(iobase + UART_REG_LINE_STATUS) & UART_REG_LINE_STATUS_THR_EMPTY); i++)
		 microdelay(10);*/
	outb(iobase + UART_REG_CHAR, c);
}

static int uartgetc(void) {
	if (!uart_devices[0].exist)
		return -1;
	ioport_t iobase = uart_devices[0].iobase;
	if (!(inb(iobase + UART_REG_LINE_STATUS) & UART_REG_LINE_STATUS_DATA_READY))
		return -1;
	return inb(iobase + UART_REG_CHAR);
}

void uart_intr(int irq) {
	if (irq == 4) {
		consoleintr(uartgetc);
	}
}
