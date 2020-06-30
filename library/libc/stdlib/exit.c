/*
 * exit function
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

#include <holeos.h>

void (*__libc_atexit_funcs[32])(void);
int __libc_atexit_count = -1;

_Noreturn void exit(int status) {
	// call atexit() registered functions
	while (__libc_atexit_count >= 0) {
		__libc_atexit_funcs[__libc_atexit_count]();
		__libc_atexit_count--;
	}
	// terminate the program
	proc_exit(status);
}
