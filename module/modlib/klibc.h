/*
 * Kernel module C Library
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

#ifndef _MODLIB_KLIBC_H
#define _MODLIB_KLIBC_H

static inline void memset(void* s, char c, int n) {
	char* ss = s;
	for (int i = 0; i < n; i++) {
		ss[i] = c;
	}
}

static inline void memcpy(void* dest, const void* src, int n) {
	char* d = dest;
	const char* s = src;
	for (int i = 0; i < n; i++) {
		d[i] = s[i];
	}
}

#endif
