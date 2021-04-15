/*
 * Kernel arch specific functions
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

#ifndef _MODLIB_ARCH_H
#define _MODLIB_ARCH_H

#include <kernsrv.h>

struct MSIMessage {
	uint64_t addr;
	uint32_t data;
};

static inline int msi_alloc_vector(struct MSIMessage* msg, void (*handler)(void*), void* private) {
	return kernsrv->msi_alloc_vector(msg, handler, private);
}

static inline void msi_free_vector(const struct MSIMessage* msg) {
	return kernsrv->msi_free_vector(msg);
}

#endif
