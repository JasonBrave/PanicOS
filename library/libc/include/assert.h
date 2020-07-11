/*
 * assert.h header
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

#ifndef _LIBC_ASSERT_H
#define _LIBC_ASSERT_H

#ifndef NDEBUG
extern _Noreturn void __libc_assert_fail(const char* file, int line, const char* func,
										 const char* expr);
#define assert(expr)                                                                   \
	if ((expr) == 0) {                                                                 \
		__libc_assert_fail(__FILE__, __LINE__, __func__, #expr);                       \
	}
#else
#define assert(expr) ((void)0)
#endif

#endif
