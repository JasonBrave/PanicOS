/*
 * Device manager
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

#include <array>
#include <cstdlib>
#include <kcall/pci.h>
#include <vector>

#include "Devices.hpp"

namespace {
	std::string ull2hexstr(unsigned long long value) {
		if (value == 0) {
			return "0";
		}
		char buf[20];
		std::string str;
		int i = 0;
		while (value) {
			unsigned int dig = value % 16;
			buf[i++] = (dig >= 10) ? (dig - 10 + 'a') : (dig + '0');
			value /= 16;
		}
		while (i--) {
			str.push_back(buf[i]);
		}
		return str;
	}

	unsigned int powof2(unsigned int power) {
		unsigned int n = 1;
		for (unsigned int i = 0; i < power; i++) {
			n *= 2;
		}
		return n;
	}
} // namespace

std::vector<DevmgrDevice> devmgr_devices;

void devices_pci_add_device(struct PCIAddr pciaddr, const struct PCIType0ConfigHeader* cfg_space,
							const struct PciKcallResource* pcires, const char* drvname) {
	DevmgrDevice pcidevice;
	pcidevice.name = "Vendor ";
	pcidevice.name += ull2hexstr(cfg_space->vendor);
	pcidevice.name += " Device ";
	pcidevice.name += ull2hexstr(cfg_space->device);
	pcidevice.dev_addr = "PCI ";
	pcidevice.dev_addr += ull2hexstr(pciaddr.bus) + ':' + ull2hexstr(pciaddr.device) + '.' +
						  ull2hexstr(pciaddr.function);
	pcidevice.driver_name = drvname;
	// add resource bars
	int num_bars = 0;
	if (cfg_space->header_type == 0 || cfg_space->header_type == 0x80) {
		num_bars = 6;
	} else if (cfg_space->header_type == 1 || cfg_space->header_type == 0x81) {
		num_bars = 2;
	}
	for (int i = 0; i < num_bars; i++) {
		if (cfg_space->bar[i] && (cfg_space->bar[i] & 1)) { // io bar
			DevmgrResource bar;
			bar.type = DevmgrResourceType::io;
			bar.addr = cfg_space->bar[i] & 0xfffffffc;
			bar.size = pcires->bar_size[i];
			pcidevice.resource.push_back(bar);
		} else if (cfg_space->bar[i] && (cfg_space->bar[i] & 6)) { // mem64 bar
			DevmgrResource bar;
			bar.type = DevmgrResourceType::memory;
			bar.addr = ((uint64_t)cfg_space->bar[i + 1] << 32) | (cfg_space->bar[i] & 0xfffffff0);
			bar.size = pcires->bar_size[i];
			pcidevice.resource.push_back(bar);
		} else if (cfg_space->bar[i]) { // mem32 bar
			DevmgrResource bar;
			bar.type = DevmgrResourceType::memory;
			bar.addr = cfg_space->bar[i] & 0xfffffff0;
			bar.size = pcires->bar_size[i];
			pcidevice.resource.push_back(bar);
		}
	}
	// add interrupts
	bool device_use_intx = true;
	const void* cfg = cfg_space;
	unsigned int cap_off = cfg_space->capptr;
	while (cap_off) {
		if (*((uint8_t*)cfg + cap_off) == 0x5) {
			uint16_t msgctl = *(uint16_t*)((uint8_t*)cfg + cap_off + 2);
			unsigned int num = powof2((msgctl >> 1) & 7);
			if (msgctl & 1) {
				uint8_t vec;
				if (msgctl & (1 << 7)) {
					vec = *(uint16_t*)((uint8_t*)cfg + cap_off + 12) & 0xff;
				} else {
					vec = *(uint16_t*)((uint8_t*)cfg + cap_off + 8) & 0xff;
				}
				DevmgrResource msires;
				msires.type = DevmgrResourceType::msi;
				msires.addr = vec;
				msires.size = num;
				pcidevice.resource.push_back(msires);
				device_use_intx = false;
			}
		} else if (*((uint8_t*)cfg + cap_off) == 0x11) {
			uint16_t msgctl = *(uint16_t*)((uint8_t*)cfg + cap_off + 2);
			if (msgctl & (1 << 15)) {
				unsigned int num = (msgctl & 0x7ff) + 1;
				DevmgrResource msixres;
				msixres.type = DevmgrResourceType::msix;
				msixres.size = num;
				pcidevice.resource.push_back(msixres);
				device_use_intx = false;
			}
		}
		cap_off = *((uint8_t*)cfg + cap_off + 1);
	}

	if (cfg_space->intr_line && device_use_intx) {
		DevmgrResource intx;
		intx.type = DevmgrResourceType::irq;
		intx.addr = cfg_space->intr_line;
		pcidevice.resource.push_back(intx);
	}

	devmgr_devices.push_back(pcidevice);
}

void devices_pci_init() {
	struct PCIType0ConfigHeader* cfg_space =
		reinterpret_cast<struct PCIType0ConfigHeader*>(std::malloc(4096));
	struct PCIAddr pciaddr;
	unsigned int pciid = pci_get_first_addr(&pciaddr);
	if (pciid == 0xffffffff) {
		std::free(cfg_space);
		return;
	}
	struct PciKcallResource pcires;
	char pcidrvname[64];
	pci_read_config(pciid, cfg_space);
	pci_get_resource(pciid, &pcires);
	pci_get_driver_name(pciid, pcidrvname);
	devices_pci_add_device(pciaddr, cfg_space, &pcires, pcidrvname);
	pciid = pci_get_next_addr(pciid, &pciaddr);
	while (pciid) {
		pci_read_config(pciid, cfg_space);
		pci_get_resource(pciid, &pcires);
		pci_get_driver_name(pciid, pcidrvname);
		devices_pci_add_device(pciaddr, cfg_space, &pcires, pcidrvname);
		pciid = pci_get_next_addr(pciid, &pciaddr);
	}
	std::free(cfg_space);
}

void devices_init() {
	// rtc
	DevmgrDevice rtc;
	rtc.name = "Real Time Clock";
	rtc.dev_addr = "ISA Bus";
	DevmgrResource rtc_io;
	rtc_io.type = DevmgrResourceType::io;
	rtc_io.addr = 0x70;
	rtc_io.size = 2;
	rtc.resource.push_back(rtc_io);
	DevmgrResource rtc_irq;
	rtc_irq.type = DevmgrResourceType::irq;
	rtc_irq.addr = 8;
	rtc.resource.push_back(rtc_irq);
	devmgr_devices.push_back(rtc);
	// uart
	DevmgrDevice uart;
	uart.name = "UART Serial Port";
	uart.dev_addr = "ISA Bus";
	DevmgrResource uart_io;
	uart_io.type = DevmgrResourceType::io;
	uart_io.addr = 0x3f8;
	uart_io.size = 8;
	uart.resource.push_back(uart_io);
	DevmgrResource uart_irq;
	uart_irq.type = DevmgrResourceType::irq;
	uart_irq.addr = 4;
	uart.resource.push_back(uart_irq);
	devmgr_devices.push_back(uart);
	// i8042
	DevmgrDevice i8042;
	i8042.name = "PS/2 Controller";
	i8042.dev_addr = "ISA Bus";
	DevmgrResource i8042_port60;
	i8042_port60.type = DevmgrResourceType::io;
	i8042_port60.addr = 0x60;
	i8042_port60.size = 1;
	i8042.resource.push_back(i8042_port60);
	DevmgrResource i8042_port64;
	i8042_port64.type = DevmgrResourceType::io;
	i8042_port64.addr = 0x64;
	i8042_port64.size = 1;
	i8042.resource.push_back(i8042_port64);
	DevmgrResource i8042_irq1;
	i8042_irq1.type = DevmgrResourceType::irq;
	i8042_irq1.addr = 1;
	i8042.resource.push_back(i8042_irq1);
	DevmgrResource i8042_irq12;
	i8042_irq12.type = DevmgrResourceType::irq;
	i8042_irq12.addr = 12;
	i8042.resource.push_back(i8042_irq12);
	devmgr_devices.push_back(i8042);
	// IO-APIC
	DevmgrDevice ioapic;
	ioapic.name = "IO Advanced Programmable Interrupt Controller";
	ioapic.dev_addr = "Chipset";
	DevmgrResource ioapic_mmio;
	ioapic_mmio.type = DevmgrResourceType::memory;
	ioapic_mmio.addr = 0xfec00000;
	ioapic_mmio.size = 4096;
	ioapic.resource.push_back(ioapic_mmio);
	devmgr_devices.push_back(ioapic);

	devices_pci_init();
}
