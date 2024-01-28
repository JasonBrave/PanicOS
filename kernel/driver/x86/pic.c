/*
 * Legacy PIC driver
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

// I/O Addresses of the two programmable interrupt controllers
#define IO_PIC1 0x20 // First (IRQs 0-7)
#define IO_PIC2 0xA0 // Second (IRQs 8-15)

void picinit(void) {
	// mask all interrupts
	outb(IO_PIC1 + 1, 0xFF);
	outb(IO_PIC2 + 1, 0xFF);

	outb(IO_PIC1, 0x11);
	outb(IO_PIC2, 0x11);

	outb(IO_PIC1 + 1, 32);
	outb(IO_PIC2 + 1, 40);

	outb(IO_PIC1 + 1, 0x04);
	outb(IO_PIC2 + 1, 0x02);

	outb(IO_PIC1 + 1, 0x01);
	outb(IO_PIC1 + 1, 0x01);
}
