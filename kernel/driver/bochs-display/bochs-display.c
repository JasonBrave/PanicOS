/*
 * QEMU bochs display interface driver
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
// https://github.com/qemu/qemu/blob/master/docs/specs/standard-vga.txt
// https://github.com/qemu/vgabios/blob/master/vbe_display_api.txt

#include <common/x86.h>
#include <defs.h>
#include <driver/pci/pci.h>
#include <hal/hal.h>

#include "bochs-display.h"

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF

#define VBE_DISPI_INDEX_ID 0x0
#define VBE_DISPI_INDEX_XRES 0x1
#define VBE_DISPI_INDEX_YRES 0x2
#define VBE_DISPI_INDEX_BPP 0x3
#define VBE_DISPI_INDEX_ENABLE 0x4
#define VBE_DISPI_INDEX_BANK 0x5
#define VBE_DISPI_INDEX_VIRT_WIDTH 0x6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 0x7
#define VBE_DISPI_INDEX_X_OFFSET 0x8
#define VBE_DISPI_INDEX_Y_OFFSET 0x9

#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_VBE_ENABLED 0x40
#define VBE_DISPI_NOCLEARMEM 0x80

#define BOCHS_DISPLAY_MMIO_OFFSET 0x500

struct BochsDisplayDevice {
	volatile uint16_t* mmio;
	phyaddr_t vram;
};

static uint16_t vbe_read(struct BochsDisplayDevice* dev, int reg) {
	if (dev->mmio) {
		return dev->mmio[reg];
	} else {
		outw(VBE_DISPI_IOPORT_INDEX, reg);
		return inw(VBE_DISPI_IOPORT_DATA);
	}
}

static void vbe_write(struct BochsDisplayDevice* dev, int reg, uint16_t val) {
	if (dev->mmio) {
		dev->mmio[reg] = val;
	} else {
		outw(VBE_DISPI_IOPORT_INDEX, reg);
		outw(VBE_DISPI_IOPORT_DATA, val);
	}
}

static phyaddr_t bochs_display_enable(void* private, int xres, int yres) {
	struct BochsDisplayDevice* dev = private;
	vbe_write(dev, VBE_DISPI_INDEX_XRES, xres);
	vbe_write(dev, VBE_DISPI_INDEX_YRES, yres);
	vbe_write(dev, VBE_DISPI_INDEX_BPP, 24);
	vbe_write(dev, VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_VBE_ENABLED);
	return dev->vram;
}

static void bochs_display_disable(void* private) {
	struct BochsDisplayDevice* dev = private;
	vbe_write(dev, VBE_DISPI_INDEX_ENABLE, 0);
	return;
}

struct FramebufferDriver bochs_display_driver = {
	.enable = bochs_display_enable,
	.disable = bochs_display_disable,
};

static void bochs_display_dev_init(struct PCIDevice* pcidev) {
	const struct PciAddress* addr = &pcidev->addr;
	int try_mmio;
	if (pci_read_config_reg16(addr, 0) == 0x1234 &&
		pci_read_config_reg16(addr, 2) == 0x1111) {
		try_mmio = 1;
	} else {
		try_mmio = 0;
	}

	struct BochsDisplayDevice* dev = kalloc();
	memset(dev, 0, sizeof(struct BochsDisplayDevice));

	phyaddr_t mmio_bar = pci_read_bar(addr, 2);
	if (mmio_bar && try_mmio) {
		dev->mmio = (volatile uint16_t*)(mmio_bar + BOCHS_DISPLAY_MMIO_OFFSET);
	}
	dev->vram = pci_read_bar(addr, 0);

	cprintf("[bochs-display] ver %x MMIO %x FB %x\n", vbe_read(dev, VBE_DISPI_INDEX_ID),
			mmio_bar, dev->vram);

	hal_display_register_device("bochs-display", dev, &bochs_display_driver);
}

struct PCIDeviceID bochs_display_device_id[] = {
	{0x1234, 0x1111}, // QEMU
	{0x80ee, 0xbeef}, // VirtualBox
	{},
};

struct PCIDriver bochs_display_pci_driver = {
	.name = "bochs-display",
	.match_table = bochs_display_device_id,
	.init = bochs_display_dev_init,
};

void bochs_display_init(void) {
	pci_register_driver(&bochs_display_pci_driver);
}
