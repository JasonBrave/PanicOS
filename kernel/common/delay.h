/*
 * Millisecond and microsecond delay
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

#ifndef _COMMON_DELAY_H
#define _COMMON_DELAY_H

#include <common/types.h>

#define _X86GPRINTRIN_H_INCLUDED
#include <ia32intrin.h>

static inline void mdelay(uint64_t ms) {
	uint64_t end = __rdtsc() + ms * 2000 * 1000;
	while (__rdtsc() < end) {
	}
}

static inline void udelay(uint64_t us) {
	uint64_t end = __rdtsc() + us * 2000;
	while (__rdtsc() < end) {
	}
}

#endif
