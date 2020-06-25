/*
 * Multiprocessor support
 *
 * This file is part of HoleOS.
 *
 * HoleOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HoleOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HoleOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common/x86.h>
#include <core/mmu.h>
#include <core/mp.h>
#include <core/proc.h>
#include <core/traps.h>
#include <defs.h>
#include <memlayout.h>
#include <param.h>

struct cpu cpus[NCPU];
int ncpu;
unsigned char ioapicid;

static unsigned char sum(unsigned char* addr, int len) {
	int i, sum;

	sum = 0;
	for (i = 0; i < len; i++)
		sum += addr[i];
	return sum;
}

// Look for an MP structure in the len bytes at addr.
static struct mp* mpsearch1(unsigned int a, int len) {
	unsigned char *e, *p, *addr;

	addr = P2V(a);
	e = addr + len;
	for (p = addr; p < e; p += sizeof(struct mp))
		if (memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
			return (struct mp*)p;
	return 0;
}

// Search for the MP Floating Pointer Structure, which according to the
// spec is in one of the following three locations:
// 1) in the first KB of the EBDA;
// 2) in the last KB of system base memory;
// 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
static struct mp* mpsearch(void) {
	unsigned char* bda;
	unsigned int p;
	struct mp* mp;

	bda = (unsigned char*)P2V(0x400);
	if ((p = ((bda[0x0F] << 8) | bda[0x0E]) << 4)) {
		if ((mp = mpsearch1(p, 1024)))
			return mp;
	} else {
		p = ((bda[0x14] << 8) | bda[0x13]) * 1024;
		if ((mp = mpsearch1(p - 1024, 1024)))
			return mp;
	}
	return mpsearch1(0xF0000, 0x10000);
}

// Search for an MP configuration table.  For now,
// don't accept the default configurations (physaddr == 0).
// Check for correct signature, calculate the checksum and,
// if correct, check the version.
// To do: check extended table checksum.
static struct mpconf* mpconfig(struct mp** pmp) {
	struct mpconf* conf;
	struct mp* mp;

	if ((mp = mpsearch()) == 0 || mp->physaddr == 0)
		return 0;
	conf = (struct mpconf*)P2V((unsigned int)mp->physaddr);
	if (memcmp(conf, "PCMP", 4) != 0)
		return 0;
	if (conf->version != 1 && conf->version != 4)
		return 0;
	if (sum((unsigned char*)conf, conf->length) != 0)
		return 0;
	*pmp = mp;
	return conf;
}

void mpinit(void) {
	unsigned char *p, *e;
	int ismp;
	struct mp* mp;
	struct mpconf* conf;
	struct mpproc* proc;
	struct mpioapic* ioapic;

	if ((conf = mpconfig(&mp)) == 0)
		panic("Expect to run on an SMP");
	ismp = 1;
	lapic = (unsigned int*)conf->lapicaddr;
	for (p = (unsigned char*)(conf + 1), e = (unsigned char*)conf + conf->length;
		 p < e;) {
		switch (*p) {
		case MPPROC:
			proc = (struct mpproc*)p;
			if (ncpu < NCPU) {
				cpus[ncpu].apicid = proc->apicid; // apicid may differ from ncpu
				ncpu++;
			}
			p += sizeof(struct mpproc);
			continue;
		case MPIOAPIC:
			ioapic = (struct mpioapic*)p;
			ioapicid = ioapic->apicno;
			p += sizeof(struct mpioapic);
			continue;
		case MPBUS:
		case MPIOINTR:
		case MPLINTR:
			p += 8;
			continue;
		default:
			ismp = 0;
			break;
		}
	}
	if (!ismp)
		panic("Didn't find a suitable machine");

	if (mp->imcrp) {
		// Bochs doesn't support IMCR, so this doesn't run on Bochs.
		// But it would on real hardware.
		outb(0x22, 0x70); // Select IMCR
		outb(0x23, inb(0x23) | 1); // Mask external interrupts.
	}
}
