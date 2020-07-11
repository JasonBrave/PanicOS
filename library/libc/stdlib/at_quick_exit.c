/*
 * at_quick_exit function
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

extern void (*__libc_atquickexit_funcs[])(void);
extern int __libc_atquickexit_count;

#define ATQUICKEXIT_MAX 32

int at_quick_exit(void (*func)(void)) {
	if (__libc_atquickexit_count >= ATQUICKEXIT_MAX - 1) {
		return -1;
	}
	__libc_atquickexit_count++;
	__libc_atquickexit_funcs[__libc_atquickexit_count] = func;
	return 0;
}
