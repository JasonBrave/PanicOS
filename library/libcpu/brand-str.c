/*
 * Read CPU brand string
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

#include "cpuinfo.h"

char* cpu_get_brand_string(char* brand_str) {

	if (cpu_get_max_ext_cpuid() < 0x80000004) {
		brand_str[0] = '\0';
		return brand_str;
	}

	unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
	__get_cpuid(0x80000002, &eax, &ebx, &ecx, &edx);

	brand_str[0] = eax & 0xff;
	brand_str[1] = (eax >> 8) & 0xff;
	brand_str[2] = (eax >> 16) & 0xff;
	brand_str[3] = (eax >> 24) & 0xff;

	brand_str[4] = ebx & 0xff;
	brand_str[5] = (ebx >> 8) & 0xff;
	brand_str[6] = (ebx >> 16) & 0xff;
	brand_str[7] = (ebx >> 24) & 0xff;

	brand_str[8] = ecx & 0xff;
	brand_str[9] = (ecx >> 8) & 0xff;
	brand_str[10] = (ecx >> 16) & 0xff;
	brand_str[11] = (ecx >> 24) & 0xff;

	brand_str[12] = edx & 0xff;
	brand_str[13] = (edx >> 8) & 0xff;
	brand_str[14] = (edx >> 16) & 0xff;
	brand_str[15] = (edx >> 24) & 0xff;

	__get_cpuid(0x80000003, &eax, &ebx, &ecx, &edx);

	brand_str[16] = eax & 0xff;
	brand_str[17] = (eax >> 8) & 0xff;
	brand_str[18] = (eax >> 16) & 0xff;
	brand_str[19] = (eax >> 24) & 0xff;

	brand_str[20] = ebx & 0xff;
	brand_str[21] = (ebx >> 8) & 0xff;
	brand_str[22] = (ebx >> 16) & 0xff;
	brand_str[23] = (ebx >> 24) & 0xff;

	brand_str[24] = ecx & 0xff;
	brand_str[25] = (ecx >> 8) & 0xff;
	brand_str[26] = (ecx >> 16) & 0xff;
	brand_str[27] = (ecx >> 24) & 0xff;

	brand_str[28] = edx & 0xff;
	brand_str[29] = (edx >> 8) & 0xff;
	brand_str[30] = (edx >> 16) & 0xff;
	brand_str[31] = (edx >> 24) & 0xff;

	__get_cpuid(0x80000004, &eax, &ebx, &ecx, &edx);

	brand_str[32] = eax & 0xff;
	brand_str[33] = (eax >> 8) & 0xff;
	brand_str[34] = (eax >> 16) & 0xff;
	brand_str[35] = (eax >> 24) & 0xff;

	brand_str[36] = ebx & 0xff;
	brand_str[37] = (ebx >> 8) & 0xff;
	brand_str[38] = (ebx >> 16) & 0xff;
	brand_str[39] = (ebx >> 24) & 0xff;

	brand_str[40] = ecx & 0xff;
	brand_str[41] = (ecx >> 8) & 0xff;
	brand_str[42] = (ecx >> 16) & 0xff;
	brand_str[43] = (ecx >> 24) & 0xff;

	brand_str[44] = edx & 0xff;
	brand_str[45] = (edx >> 8) & 0xff;
	brand_str[46] = (edx >> 16) & 0xff;
	brand_str[47] = (edx >> 24) & 0xff;

	brand_str[48] = '\0';
	return brand_str;
}

char* cpu_get_vendor_string(char* vendor_str) {
	unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
	__get_cpuid(0, &eax, &ebx, &ecx, &edx);

	vendor_str[0] = ebx & 0xff;
	vendor_str[1] = (ebx >> 8) & 0xff;
	vendor_str[2] = (ebx >> 16) & 0xff;
	vendor_str[3] = (ebx >> 24) & 0xff;

	vendor_str[4] = edx & 0xff;
	vendor_str[5] = (edx >> 8) & 0xff;
	vendor_str[6] = (edx >> 16) & 0xff;
	vendor_str[7] = (edx >> 24) & 0xff;

	vendor_str[8] = ecx & 0xff;
	vendor_str[9] = (ecx >> 8) & 0xff;
	vendor_str[10] = (ecx >> 16) & 0xff;
	vendor_str[11] = (ecx >> 24) & 0xff;

	vendor_str[12] = '\0';
	return vendor_str;
}
