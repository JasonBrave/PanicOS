/*
 * I/O APIC driver
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

#include <common/types.h>
#include <defs.h>

#include "ioapic.h"

#define IOAPIC_REG_ID 0
#define IOAPIC_REG_ID_IOAPICID_SHIFT 24
#define IOAPIC_REG_VER 1
#define IOAPIC_REG_VER_IRQS_SHIFT 16
#define IOAPIC_REG_VER_VERSION_SHIFT 0
#define IOAPIC_REG_LO_MASK (1 << 16)
#define IOAPIC_REG_LO_TRIGGER_MODE_BIT 15
#define IOAPIC_REG_LO_INTR_POLARITY_BIT 13
#define IOAPIC_REG_HI_DEST_SHIFT 24

struct IOAPICMMIO {
	uint32_t index;
	uint32_t pad[3];
	uint32_t data;
} PACKED;

static struct IOAPICDevice {
	volatile struct IOAPICMMIO* mmio;
	unsigned int num_irqs;
} ioapic;

static uint32_t ioapic_read(unsigned int reg) {
	ioapic.mmio->index = reg;
	return ioapic.mmio->data;
}

static void ioapic_write(unsigned int reg, uint32_t value) {
	ioapic.mmio->index = reg;
	ioapic.mmio->data = value;
}

void ioapic_init(void) {
	ioapic.mmio = map_mmio_region(0xfec00000, 4096);
	ioapic.num_irqs = ((ioapic_read(IOAPIC_REG_VER) >> IOAPIC_REG_VER_IRQS_SHIFT) & 0xff) + 1;
	cprintf("[ioapic] ioapicid %x ver %x irqs %d\n",
			(ioapic_read(IOAPIC_REG_ID) >> IOAPIC_REG_ID_IOAPICID_SHIFT) & 0xf,
			ioapic_read(IOAPIC_REG_VER) & 0xff, ioapic.num_irqs);
	// mask all interrupts
	for (unsigned int i = 0; i < ioapic.num_irqs; i++) {
		ioapic_write(0x10 + 2 * i, IOAPIC_REG_LO_MASK | (32 + i));
		ioapic_write(0x10 + 2 * i + 1, 0);
	}
}

void ioapic_enable(int irq, unsigned int lapicid, enum IOAPICTriggerMode trigger_mode,
				   enum IOAPICIntrPinPolarity polarity) {
	ioapic_write(0x10 + 2 * irq, (trigger_mode << IOAPIC_REG_LO_TRIGGER_MODE_BIT) |
									 (polarity << IOAPIC_REG_LO_INTR_POLARITY_BIT) | (32 + irq));
	ioapic_write(0x10 + 2 * irq + 1, lapicid << IOAPIC_REG_HI_DEST_SHIFT);
}

void ioapic_disable(int irq) {
	ioapic_write(0x10 + 2 * irq, IOAPIC_REG_LO_MASK | (32 + irq));
	ioapic_write(0x10 + 2 * irq + 1, 0);
}
