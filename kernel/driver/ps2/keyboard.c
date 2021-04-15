/*
 * PS/2 Keyboard driver
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
#include <hal/hal.h>

#include "kbd.h"
#include "ps2-keyboard-map.h"
#include "ps2.h"

static unsigned int rawkbddata = 0;

static unsigned int kbd_scan_to_keycode(unsigned int data) {
	unsigned int keycode = 0;
	int rel;
	if (data & 0x80) {
		rel = 1;
		data = data & 0x7f;
	} else {
		rel = 0;
	}

	keycode = ps2_keyboard_map[data];

	if (rel) {
		return keycode | 0x100;
	} else {
		return keycode;
	}
}

void ps2_keyboard_init(void) {
	cprintf("[ps2] PS/2 Keyboard found\n");
	ioapic_enable(1, 0, IOAPIC_EDGE_TRIGGER, IOAPIC_ACTIVE_HIGH);
	inb(PS2_DATA_PORT);
}

static int kbd_getc(void) {
	static unsigned int shift;
	static unsigned char* charcode[4] = {normalmap, shiftmap, ctlmap, ctlmap};
	unsigned int data, c;

	if (rawkbddata == 0) {
		return -1;
	}

	data = rawkbddata;
	rawkbddata = 0;

	if (data == 0xE0) {
		shift |= E0ESC;
		return 0;
	} else if (data & 0x80) {
		// Key released
		data = (shift & E0ESC ? data : data & 0x7F);
		shift &= ~(shiftcode[data] | E0ESC);
		return 0;
	} else if (shift & E0ESC) {
		// Last character was an E0 escape; or with 0x80
		data |= 0x80;
		shift &= ~E0ESC;
	}

	shift |= shiftcode[data];
	shift ^= togglecode[data];
	c = charcode[shift & (CTL | SHIFT)][data];
	if (shift & CAPSLOCK) {
		if ('a' <= c && c <= 'z')
			c += 'A' - 'a';
		else if ('A' <= c && c <= 'Z')
			c += 'a' - 'A';
	}
	return c;
}

void ps2_keyboard_intr(void) {
	int stat = inb(PS2_STATUS_PORT);
	if ((stat & 1) == 0) {
		return;
	}
	rawkbddata = inb(PS2_DATA_PORT);
	if (hal_kbd_send_legacy) {
		if (rawkbddata == 0x45) {
			hal_keyboard_update(144);
		}
		consoleintr(kbd_getc);
	} else {
		hal_keyboard_update(kbd_scan_to_keycode(rawkbddata));
	}
}
