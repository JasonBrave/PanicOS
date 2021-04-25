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

#include <cpuid.h>
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

struct CPUIDFlag {
	const char* flag_abbrev;
	const char* flag_name;
	unsigned int eax, reg, bitoff; // reg eax=0,ebx=1,ecx=2,edx=3
} cpuid_flags[] = {{"PSE", "Page Size Extension", 1, 3, 3},
				   {"TSC", "Time Stamp Counter", 1, 3, 4},
				   {"MSR", "Model-specific Registers", 1, 3, 5},
				   {"PAE", "Physical Address Extension", 1, 3, 6},
				   {"MCE", "Machine Check Exception", 1, 3, 7},
				   {"APIC", "Advanced Programmable Interrupt Controller", 1, 3, 9},
				   {"SEP", "SYSENTER and SYSEXIT", 1, 3, 11},
				   {"MTRR", "Memory Type Range Registers", 1, 3, 12},
				   {"PGE", "Page Global Enable", 1, 3, 13},
				   {"MCA", "Machine Check Architecture", 1, 3, 14},
				   {"PAT", "Page Attribute Table", 1, 3, 16},
				   {"x2APIC", "x2APIC", 1, 2, 21},
				   {"SYSCALL", "SYSCALL and SYSRET", 0x80000001, 3, 11},
				   {"NX", "No-execute Bit", 0x80000001, 3, 20},
				   {"PDPE1GB", "1GiB Page", 0x80000001, 3, 26},
				   {"LM", "Long mode", 0x80000001, 3, 29}};

int main() {
	print_vendor_string();
	unsigned int max_cpuid = cpu_get_max_cpuid();
	unsigned int max_ext_cpuid = cpu_get_max_ext_cpuid();
	printf("Max CPUID : %xh\n", max_cpuid);
	printf("Max Extended CPUID : %xh\n", max_ext_cpuid);
	print_brand_string();

	if (cpu_get_max_ext_cpuid() >= 0x80000008) {
		printf("Address Sizes: Physical %d bits, Virtual %d bits\n",
			   cpu_get_physical_address_size(), cpu_get_virtual_address_size());
	}

	for (unsigned int i = 0; i < (sizeof(cpuid_flags) / sizeof(struct CPUIDFlag)); i++) {
		if (cpuid_flags[i].eax < 0x80000000 && cpuid_flags[i].eax > max_cpuid) {
			continue;
		} else if (cpuid_flags[i].eax >= 0x80000000 && cpuid_flags[i].eax > max_ext_cpuid) {
			continue;
		}
		unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
		__get_cpuid(cpuid_flags[i].eax, &eax, &ebx, &ecx, &edx);
		unsigned int thereg = 0;
		switch (cpuid_flags[i].reg) {
		case 0:
			thereg = eax;
			break;
		case 1:
			thereg = ebx;
			break;
		case 2:
			thereg = ecx;
			break;
		case 3:
			thereg = edx;
			break;
		}
		if (thereg & (1 << cpuid_flags[i].bitoff)) {
			printf("%s (%s): yes\n", cpuid_flags[i].flag_abbrev, cpuid_flags[i].flag_name);
		} else {
			printf("%s (%s): no\n", cpuid_flags[i].flag_abbrev, cpuid_flags[i].flag_name);
		}
	}

	return 0;
}
