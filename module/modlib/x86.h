/*
 * Kernel module x86 helper functions
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

#ifndef _MODLIB_X86_H
#define _MODLIB_X86_H

#include <kernel-types.h>

static inline uint8_t inb(ioport_t port) {
	uint8_t data;
	__asm__ volatile("in %1,%0" : "=a"(data) : "d"(port));
	return data;
}

static inline uint16_t inw(ioport_t port) {
	uint16_t data;
	__asm__ volatile("in %1,%0" : "=a"(data) : "d"(port));
	return data;
}

static inline uint32_t indw(ioport_t port) {
	uint32_t data;
	__asm__ volatile("in %1,%0" : "=a"(data) : "d"(port));
	return data;
}

static inline void insw(ioport_t port, void* addr, int cnt) {
	__asm__ volatile("cld; rep insw"
					 : "=D"(addr), "=c"(cnt)
					 : "d"(port), "0"(addr), "1"(cnt)
					 : "memory", "cc");
}

static inline void outb(ioport_t port, uint8_t data) {
	__asm__ volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline void outw(ioport_t port, uint16_t data) {
	__asm__ volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline void outdw(ioport_t port, uint32_t data) {
	__asm__ volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline void outsw(ioport_t port, const void* addr, int cnt) {
	__asm__ volatile("cld; rep outsw"
					 : "=S"(addr), "=c"(cnt)
					 : "d"(port), "0"(addr), "1"(cnt)
					 : "cc");
}

#endif
