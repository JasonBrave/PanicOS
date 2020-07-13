/*
 * lscpu program
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

#include <libcpu/cpuinfo.h>
#include <stdio.h>

void print_vendor_string() {
	char vendor[13];
	printf("Vendor String : %s\n", cpu_get_vendor_string(vendor));
}

void print_brand_string() {
	char brand[50];
	printf("CPU Brand String : %s\n", cpu_get_brand_string(brand));
}

int main() {
	print_vendor_string();
	printf("Max CPUID : %xh\n", cpu_get_max_cpuid());
	printf("Max Extended CPUID : %xh\n", cpu_get_max_ext_cpuid());
	print_brand_string();

	if (cpu_get_max_ext_cpuid() >= 0x80000008) {
		printf("Address Sizes: Physical %d bits, Virtual %d bits\n",
			   cpu_get_physical_address_size(), cpu_get_virtual_address_size());
	}

	return 0;
}
