/*
 * x86 platform driver
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

#include <defs.h>
#include <driver/pci/pci.h>
#include <driver/ps2/ps2.h>
#include <driver/x86/ioapic.h>
#include <driver/x86/rtc.h>
#include <driver/x86/uart.h>

extern void intel_pcie_mmcfg_init(const struct PciAddress* host_bridge_addr);
extern void picinit(void);

void platform_init(void) {
	// onboard devices
	picinit();
	ioapic_init();
	uart_init();
	rtc_init();
	ps2_keyboard_init();
	ps2_mouse_init();
	const struct PciAddress pci_host_addr = {0, 0, 0};
	if ((pci_read_config_reg32(&pci_host_addr, 0) == 0x8086) &&
		(pci_read_config_reg32(&pci_host_addr, 0x60) != 0)) {
		intel_pcie_mmcfg_init(&pci_host_addr);
	}
	pci_init();
}
