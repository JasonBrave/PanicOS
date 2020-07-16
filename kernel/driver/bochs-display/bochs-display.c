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

#include <defs.h>
#include <driver/pci/pci.h>

#include "bochs-display.h"

#define BOCHS_DISPLAY_MMIO_OFFSET 0x500
#define BOCHS_DISPLAY_ENABLED 0x01
#define BOCHS_DISPLAY_LFB_ENABLED 0x40

struct BochsDisplay bochs_display;

void bochs_display_init(void) {
	memset(&bochs_display, 0, sizeof(bochs_display));

	struct PciAddress addr;
	if (pci_find_device(&addr, 0x1234, 0x1111)) {
		cprintf("[bochs-display] Display at PCI %d:%d.%d\n", addr.bus, addr.device,
				addr.function);

		bochs_display.address = addr;
		bochs_display.mmio = (void*)pci_read_bar(&addr, 2) + BOCHS_DISPLAY_MMIO_OFFSET;
	}
}

phyaddr_t bochs_display_enable(int xres, int yres) {
	if (!bochs_display.mmio) {
		panic("no display adapter");
	}
	volatile struct BochsDisplayMMIO* mmio = bochs_display.mmio;
	mmio->xres = xres;
	mmio->yres = yres;
	mmio->bpp = 24;
	mmio->enable = BOCHS_DISPLAY_ENABLED | BOCHS_DISPLAY_LFB_ENABLED;
	return pci_read_bar(&bochs_display.address, 0);
}
