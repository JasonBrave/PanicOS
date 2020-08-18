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

#define EDU_USE_MSI

#define EDU_VENDOR 0x1234
#define EDU_DEVICE 0x11e8

struct EduDeviceMMIO {
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

struct EduDevice {
	volatile struct EduDeviceMMIO* mmio;
};

void edu_intx_intr(struct PCIDevice* pcidev) {
	struct EduDevice* dev = pcidev->private;
	cprintf("[edu] INTx intr %d\n", dev->mmio->intr_status);
	dev->mmio->intr_ack = dev->mmio->intr_status;
}

#ifdef EDU_USE_MSI

void edu_msi_intr(void* private) {
	struct EduDevice* dev = private;
	cprintf("[edu] MSI intr %d\n", dev->mmio->intr_status);
	dev->mmio->intr_ack = dev->mmio->intr_status;
}

#endif

void edu_init_dev(struct PCIDevice* pcidev) {
	const struct PciAddress* addr = &pcidev->addr;
	struct EduDevice* dev = kalloc();
	dev->mmio = (void*)pci_read_bar(addr, 0);
	cprintf("[edu] Version %x\n", dev->mmio->id);
	// legacy interrupt handler
	pcidev->private = dev;
	pci_register_intr_handler(pcidev, edu_intx_intr);
// set message signaled interrupt
#ifdef EDU_USE_MSI
	int msi_vec = pci_msi_alloc_vector(edu_msi_intr, dev);
	if (!msi_vec) {
		cprintf("[edu] out of MSI interrupt vector\n");
	} else {
		pci_msi_enable(addr, msi_vec, 0);
	}
#endif
	// try factorial
	dev->mmio->factorial = 10;
	while (dev->mmio->status & 1) {
	}
	cprintf("[edu] Factorial of 10 is %d\n", dev->mmio->factorial);
	// try to raise an interrupt
	dev->mmio->intr_raise = 123;
}

const struct PCIDeviceID edu_device_table[] = {
	{EDU_VENDOR, EDU_DEVICE},
	{},
};

struct PCIDriver edu_driver = {
	.name = "qemu-edu",
	.match_table = edu_device_table,
	.init = edu_init_dev,
};

void edu_init(void) {
	pci_register_driver(&edu_driver);
}
