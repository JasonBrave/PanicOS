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
#include <core/kbd.h>
#include <defs.h>
#include <proc/kcall.h>

#include "ps2-keyboard-map.h"

static unsigned int kbddata = 0;

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

int kbdgetc(void) {
	static unsigned int shift;
	static unsigned char* charcode[4] = {normalmap, shiftmap, ctlmap, ctlmap};
	unsigned int st, data, c;

	st = inb(KBSTATP);
	if ((st & KBS_DIB) == 0)
		return -1;
	data = inb(KBDATAP);
	kbddata = kbd_scan_to_keycode(data);

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

void kbdintr(void) {
	consoleintr(kbdgetc);
}

int kbd_kcall_handler(unsigned int ptr) {
	unsigned int* k = (unsigned int*)ptr;
	*k = kbddata;
	kbddata = 0;
	return 0;
}

void kbdinit(void) {
	cprintf("[kbd] Keyboard found\n");
	kcall_set("keyboard", kbd_kcall_handler);
}
