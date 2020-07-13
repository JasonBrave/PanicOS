/*
 * Read max CPUID number
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

#include <cpuid.h>

unsigned int cpu_get_max_cpuid(void) {
	unsigned int eax = 0, ebx, ecx, edx;
	__get_cpuid(0, &eax, &ebx, &ecx, &edx);
	return eax;
}

unsigned int cpu_get_max_ext_cpuid(void) {
	unsigned int eax = 0, ebx, ecx, edx;
	__get_cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
	return eax;
}
