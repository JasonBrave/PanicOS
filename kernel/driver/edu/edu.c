/*
 * QEMU edu device driver
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

// Reference:
// https://github.com/qemu/qemu/blob/master/docs/specs/edu.txt

#include <defs.h>
#include <driver/pci/pci.h>

struct PciAddress edu_addr;

void edu_init(void);
void edu_init_dev(const struct PciAddress* addr);
void edu_intr(const struct PciAddress* addr);

#define EDU_VENDOR 0x1234
#define EDU_DEVICE 0x11e8

struct EduDeviceMmio {
	const uint32_t id;
	uint32_t liveness;
	uint32_t factorial;
	uint8_t res[0x14];
	uint32_t status;
	const uint32_t intr_status;
	uint8_t res0[0x38];
	uint32_t intr_raise;
	uint32_t intr_ack;
} PACKED;

void edu_init(void) {
	edu_addr.bus = 0;
	edu_addr.device = 0;
	edu_addr.function = 0;

	struct PciAddress addr;
	if (pci_find_device(&addr, EDU_VENDOR, EDU_DEVICE)) {
		edu_init_dev(&addr);
		edu_addr = addr;
		while (pci_next_device(&addr, EDU_VENDOR, EDU_DEVICE)) {
			edu_init_dev(&addr);
		}
	}
}

void edu_init_dev(const struct PciAddress* addr) {
	pci_register_intr_handler(addr, edu_intr);
	volatile struct EduDeviceMmio* edu = (void*)pci_read_bar(addr, 0);
	cprintf("[edu] Version %x\n", edu->id);
	// try factorial
	edu->factorial = 10;
	while (edu->status & 1) {
	}
	cprintf("[edu] Factorial of 10 is %d\n", edu->factorial);
	// try to raise an interrupt
	edu->intr_raise = 123;
}

void edu_intr(const struct PciAddress* addr) {
	volatile struct EduDeviceMmio* edu = (void*)pci_read_bar(addr, 0);
	cprintf("[edu] intr %d\n", edu->intr_status);
	edu->intr_ack = edu->intr_status;
}
