/*
 * Kernel module memory helper functions
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

#ifndef _MODLIB_MEMORY_H
#define _MODLIB_MEMORY_H

#include <kernsrv.h>

static inline void* pgalloc(unsigned int num_pages) {
	return kernsrv->pgalloc(num_pages);
}

static inline void pgfree(void* ptr, unsigned int num_pages) {
	return kernsrv->pgfree(ptr, num_pages);
}

static inline void* map_mmio_region(phyaddr_t phyaddr, size_t size) {
	return kernsrv->map_mmio_region(phyaddr, size);
}

static inline void* map_ram_region(phyaddr_t phyaddr, size_t size) {
	return kernsrv->map_ram_region(phyaddr, size);
}

static inline void* map_rom_region(phyaddr_t phyaddr, size_t size) {
	return kernsrv->map_rom_region(phyaddr, size);
}

static inline void* kalloc(void) {
	return pgalloc(1);
}
static inline void kfree(void* ptr) {
	return pgfree(ptr, 1);
}

#endif
