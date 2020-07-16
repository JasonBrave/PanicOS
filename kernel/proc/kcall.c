/*
 * Kernel calls
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

#include <defs.h>

#include "kcall.h"

struct KCallTable kcall_table[256];

static int kcall_true(unsigned int arg) {
	return 123;
}

void kcall_init(void) {
	memset(kcall_table, 0, sizeof(kcall_table));
	kcall_set("true", kcall_true);
}

void kcall_set(const char* name, int (*handler)(unsigned int)) {
	for (int i = 0; i < 256; i++) {
		if (strncmp(kcall_table[i].name, name, 32) == 0) {
			if (handler) {
				kcall_table[i].handler = handler;
			} else {
				kcall_table[i].name[0] = '\0';
				kcall_table[i].handler = 0;
			}
			return;
		} else if (kcall_table[i].handler == 0) {
			strncpy(kcall_table[i].name, name, 32);
			kcall_table[i].handler = handler;
			return;
		}
	}
	panic("out ouf kcall table");
}

int kcall(const char* name, unsigned int arg) {
	for (int i = 0; i < 256; i++) {
		if (strncmp(kcall_table[i].name, name, 32) == 0) {
			return kcall_table[i].handler(arg);
		}
	}
	return -1;
}
