/*
 * Serial port
 *
 * This file is part of HoleOS.
 *
 * HoleOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HoleOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HoleOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common/x86.h>
#include <core/mmu.h>
#include <core/proc.h>
#include <core/traps.h>
#include <defs.h>
#include <memlayout.h>
#include <param.h>

#define COM1 0x3f8

static int uart; // is there a uart?

void uartinit(void) {
	// Turn off the FIFO
	outb(COM1 + 2, 0);

	// 9600 baud, 8 data bits, 1 stop bit, parity off.
	outb(COM1 + 3, 0x80); // Unlock divisor
	outb(COM1 + 0, 115200 / 9600);
	outb(COM1 + 1, 0);
	outb(COM1 + 3, 0x03); // Lock divisor, 8 data bits.
	outb(COM1 + 4, 0);
	outb(COM1 + 1, 0x01); // Enable receive interrupts.

	// If status is 0xFF, no serial port.
	if (inb(COM1 + 5) == 0xFF)
		return;
	uart = 1;

	// Acknowledge pre-existing interrupt conditions;
	// enable interrupts.
	inb(COM1 + 2);
	inb(COM1 + 0);
	ioapicenable(IRQ_COM1, 0);
}

void uartputc(int c) {
	int i;

	if (!uart)
		return;
	for (i = 0; i < 128 && !(inb(COM1 + 5) & 0x20); i++)
		microdelay(10);
	outb(COM1 + 0, c);
}

static int uartgetc(void) {
	if (!uart)
		return -1;
	if (!(inb(COM1 + 5) & 0x01))
		return -1;
	return inb(COM1 + 0);
}

void uartintr(void) {
	consoleintr(uartgetc);
}
