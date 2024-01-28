/*
 * MP Table structures
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

#ifndef _MP_H
#define _MP_H

#include <common/types.h>

struct mp { // floating pointer
	unsigned char signature[4]; // "_MP_"
	void* physaddr; // phys addr of MP config table
	unsigned char length; // 1
	unsigned char specrev; // [14]
	unsigned char checksum; // all bytes must add up to 0
	unsigned char type; // MP system config type
	unsigned char imcrp;
	unsigned char reserved[3];
};

struct mpconf { // configuration table header
	unsigned char signature[4]; // "PCMP"
	unsigned short length; // total table length
	unsigned char version; // [14]
	unsigned char checksum; // all bytes must add up to 0
	unsigned char product[20]; // product id
	unsigned int* oemtable; // OEM table pointer
	unsigned short oemlength; // OEM table length
	unsigned short entry; // entry count
	unsigned int* lapicaddr; // address of local APIC
	unsigned short xlength; // extended table length
	unsigned char xchecksum; // extended table checksum
	unsigned char reserved;
};

struct mpproc { // processor table entry
	unsigned char type; // entry type (0)
	unsigned char apicid; // local APIC id
	unsigned char version; // local APIC verison
	unsigned char flags; // CPU flags
#define MPBOOT 0x02 // This proc is the bootstrap processor.
	unsigned char signature[4]; // CPU signature
	unsigned int feature; // feature flags from CPUID instruction
	unsigned char reserved[8];
};

struct mpioapic { // I/O APIC table entry
	uint8_t type; // entry type (2)
	uint8_t id; // I/O APIC id
	uint8_t version; // I/O APIC version
	uint8_t flags; // I/O APIC flags
	uint32_t addr; // I/O APIC address
};

// Table entry types
#define MPPROC 0x00 // One per processor
#define MPBUS 0x01 // One per bus
#define MPIOAPIC 0x02 // One per I/O APIC
#define MPIOINTR 0x03 // One per bus interrupt source
#define MPLINTR 0x04 // One per system interrupt source

#endif
