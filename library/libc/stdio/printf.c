/*
 * printf function
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

#include <panicos.h>
#include <stdarg.h>
#include <stdio.h>

static void printint(unsigned long long xx, int base, int sgn, int cpt) {
	const char* digits = cpt ? "0123456789ABCDEF" : "0123456789abcdef";
	char buf[64];
	int i, neg;
	unsigned long long x;

	neg = 0;
	if (sgn && xx < 0) {
		neg = 1;
		x = -xx;
	} else {
		x = xx;
	}

	i = 0;
	do {
		buf[i++] = digits[x % base];
	} while ((x /= base) != 0);
	if (neg)
		buf[i++] = '-';

	while (--i >= 0)
		putchar(buf[i]);
}

int printf(const char* restrict fmt, ...) {
	int c, i, state;

	state = 0;
	va_list args;
	va_start(args, fmt);

	for (i = 0; fmt[i]; i++) {
		c = fmt[i] & 0xff;
		if (state == 0) {
			if (c == '%') {
				state = '%';
			} else {
				putchar(c);
			}
		} else if (state == '%') {
			state = 0;
			if (c == '%') {
				putchar(c);
			} else if (c == 'c') {
				putchar(va_arg(args, int));
			} else if (c == 's') {
				const char* s = va_arg(args, const char*);
				if (s == 0)
					s = "(null)";
				while (*s != 0) {
					putchar(*s);
					s++;
				}
			} else if (c == 'd' || c == 'i') {
				printint(va_arg(args, int), 10, 1, 0);
			} else if (c == 'x') {
				printint(va_arg(args, unsigned int), 16, 0, 0);
			} else if (c == 'X') {
				printint(va_arg(args, unsigned int), 16, 0, 1);
			} else if (c == 'u') {
				printint(va_arg(args, unsigned int), 10, 0, 0);
			} else if (c == 'p') {
				printint((unsigned int)va_arg(args, void*), 16, 0, 0);
			} else if (c == 'l') {
				state = 'l';
			} else {
				// Unknown % sequence.  Print it to draw attention.
				putchar('%');
				putchar(c);
			}
		} else if (state == 'l') {
			state = 0;
			if (c == 'd' || c == 'i') {
				printint(va_arg(args, long), 10, 1, 0);
			} else if (c == 'x') {
				printint(va_arg(args, unsigned long), 16, 0, 0);
			} else if (c == 'X') {
				printint(va_arg(args, unsigned long), 16, 0, 1);
			} else if (c == 'u') {
				printint(va_arg(args, unsigned long), 10, 0, 0);
			} else if (c == 'l') {
				state = 'L';
			}
		} else if (state == 'L') {
			state = 0;
			if (c == 'd' || c == 'i') {
				printint(va_arg(args, long long), 10, 1, 0);
			} else if (c == 'x') {
				printint(va_arg(args, unsigned long long), 16, 0, 0);
			} else if (c == 'X') {
				printint(va_arg(args, unsigned long long), 16, 0, 1);
			} else if (c == 'u') {
				printint(va_arg(args, unsigned long long), 10, 0, 0);
			}
		}
	}
	return 0;
}
