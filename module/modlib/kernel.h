/*
 * Kernel module library
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

#ifndef _MODLIB_KERNEL_H
#define _MODLIB_KERNEL_H

#include <kernsrv.h>

#define cprintf(format, ...) kernsrv->cprintf(format, ##__VA_ARGS__);

static inline void panic(const char* s) {
	return kernsrv->panic(s);
}

static inline void sleep(void* chan, struct spinlock* lock) {
	return kernsrv->sleep(chan, lock);
}

static inline void wakeup(void* chan) {
	return kernsrv->wakeup(chan);
}

static inline void initlock(struct spinlock* lock, const char* name) {
	return kernsrv->initlock(lock, name);
}

static inline void acquire(struct spinlock* lock) {
	return kernsrv->acquire(lock);
}

static inline void release(struct spinlock* lock) {
	return kernsrv->release(lock);
}

#endif
