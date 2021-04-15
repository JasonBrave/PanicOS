/*
 * PS/2 Mouse driver
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

#include <common/types.h>
#include <common/x86.h>
#include <defs.h>
#include <driver/x86/ioapic.h>
#include <hal/hal.h>

#include "ps2.h"

static enum PS2MouseType ps2_mouse_type;
static uint8_t mouse_buffer[4];
static int mouse_phase = 0;

static void ps2_wait(void) {
	while (inb(PS2_STATUS_PORT) & 2) {
	}
}

static int ps2_mouse_write(int data) {
	ps2_wait();
	outb(PS2_COMMAND_PORT, PS2_CMD_WRITE_SECOND);
	ps2_wait();
	outb(PS2_DATA_PORT, data);
	ps2_wait();
	return inb(PS2_DATA_PORT);
}

static void ps2_mouse_set_rate(int rate) {
	ps2_mouse_write(PS2_MOUSE_CMD_SET_SAMPLE_RATE);
	ps2_mouse_write(rate);
}

static int ps2_mouse_identify(void) {
	ps2_mouse_write(PS2_MOUSE_CMD_MOUSE_ID);
	ps2_wait();
	return inb(PS2_DATA_PORT);
}

static void ps2_mouse_enable_scroll(void) {
	ps2_mouse_set_rate(200);
	ps2_mouse_set_rate(100);
	ps2_mouse_set_rate(80);
}

// 0 for first port, 1 for second port
static void ps2_enable_interrupt(int port) {
	ps2_wait();
	outb(PS2_COMMAND_PORT, PS2_CMD_READ_CFG);
	unsigned char c = inb(PS2_DATA_PORT);
	c = c | (1 << port);
	ps2_wait();
	outb(PS2_COMMAND_PORT, PS2_CMD_WRITE_CFG);
	outb(PS2_DATA_PORT, c);
}

void ps2_mouse_init(void) {
	// enable second PS/2 port
	ps2_wait();
	outb(PS2_COMMAND_PORT, PS2_CMD_ENABLE_SECOND);
	// disable mouse scanning
	ps2_mouse_write(PS2_MOUSE_CMD_DISABLE_REPORT);
	// enable scroll wheel
	ps2_mouse_enable_scroll();
	// mouse id
	if (ps2_mouse_identify() == 3) {
		cprintf("[ps2] PS/2 Mouse with scroll wheel found\n");
		ps2_mouse_type = PS2_MOUSE_WITH_WHEEL;
	} else {
		cprintf("[ps2] PS/2 Mouse without scroll wheel found\n");
		ps2_mouse_type = PS2_MOUSE_WITHOUT_WHEEL;
	}
	// enable second PS/2 port interrupt
	ps2_enable_interrupt(1);
	// make mouse start report event
	ps2_mouse_write(PS2_MOUSE_CMD_ENABLE_REPORT);
	// enable mouse IRQ
	ioapic_enable(12, 0, IOAPIC_EDGE_TRIGGER, IOAPIC_ACTIVE_HIGH);
	// flush buffer
	inb(PS2_DATA_PORT);
}

void ps2_mouse_intr(void) {
	mouse_buffer[mouse_phase] = inb(PS2_DATA_PORT);
	mouse_phase++;
	if (ps2_mouse_type == PS2_MOUSE_WITH_WHEEL) {
		if (mouse_phase == 4) {
			hal_mouse_update(mouse_buffer[0] << 24 | mouse_buffer[1] << 16 | mouse_buffer[2] << 8 |
							 mouse_buffer[3]);
			mouse_phase = 0;
		}
	} else if (ps2_mouse_type == PS2_MOUSE_WITHOUT_WHEEL) {
		if (mouse_phase == 3) {
			hal_mouse_update(mouse_buffer[0] << 24 | mouse_buffer[1] << 16 | mouse_buffer[2] << 8);
			mouse_phase = 0;
		}
	}
}
