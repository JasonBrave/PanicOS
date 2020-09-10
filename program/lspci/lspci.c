/*
 * lspci program
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

#include <panicos.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int verbose_level = 0;

struct PciKcall {
#define PCI_KCALL_OP_FIRST_ADDR 0
#define PCI_KCALL_OP_NEXT_ADDR 1
#define PCI_KCALL_OP_READ_CONFIG 2
#define PCI_KCALL_OP_DRIVER_NAME 3
	unsigned int op;
	unsigned int id;
	unsigned int bus, device, function;
	void* ptr;
};

struct PCIConfigHeader {
	uint16_t vendor, device;
	uint16_t command, status;
	uint8_t revision, progif, subclass, class;
	uint8_t cache_line, lat_timer, header_type, bist;
	uint32_t bar[6];
	uint32_t cardbus_cis;
	uint16_t subsys_vendor, subsys_device;
	uint32_t rombar;
	uint8_t capptr, res[7];
	uint8_t intr_line, intr_pin, min_gnt, max_lat;
} __attribute__((packed));

static const char* pci_class_to_str(uint8_t class, uint8_t subclass, uint8_t progif) {
	// storage controllers
	if ((class == 1) && (subclass == 0)) {
		return "SCSI Controller";
	} else if ((class == 1) && (subclass == 1)) {
		return "IDE Controller";
	} else if ((class == 1) && (subclass == 2)) {
		return "Floppy Disk Controller";
	} else if ((class == 1) && (subclass == 4)) {
		return "RAID Controller";
	} else if ((class == 1) && (subclass == 5)) {
		return "ATA Controller";
	} else if ((class == 1) && (subclass == 6) && (progif == 0)) {
		return "SATA Controller (Vendor Specific)";
	} else if ((class == 1) && (subclass == 6) && (progif == 1)) {
		return "AHCI SATA Controller";
	} else if ((class == 1) && (subclass == 7)) {
		return "SAS Controller";
	} else if ((class == 1) && (subclass == 8) && (progif == 1)) {
		return "NVMHCI";
	} else if ((class == 1) && (subclass == 8) && (progif == 2)) {
		return "NVM Express";
	} else if (class == 1) {
		return "Storage Controller";
		// network controller
	} else if ((class == 2) && (subclass == 0)) {
		return "Ethernet Controller";
		// display controller
	} else if ((class == 3) && (subclass == 0)) {
		return "VGA Controller";
	} else if ((class == 3) && (subclass == 1)) {
		return "XGA Controller";
	} else if ((class == 3) && (subclass == 2)) {
		return "3D Controller";
	} else if (class == 3) {
		return "Display Controller";
		// multimedia device
	} else if ((class == 4) && (subclass == 0)) {
		return "Video Device";
	} else if ((class == 4) && (subclass == 1)) {
		return "Audio Device";
	} else if ((class == 4) && (subclass == 3)) {
		return "HD Audio Controller";
	} else if (class == 4) {
		return "Multimedia Device";
		// bridge device
	} else if ((class == 6) && (subclass == 0)) {
		return "Host Bridge";
	} else if ((class == 6) && (subclass == 1)) {
		return "ISA Bridge";
	} else if ((class == 6) && (subclass == 4) && (progif == 0)) {
		return "PCI Bridge";
	} else if ((class == 6) && (subclass == 4) && (progif == 1)) {
		return "PCI Bridge (Subtractive Decode)";
	} else if (class == 6) {
		return "Bridge Device";
		// simple communication device
	} else if ((class == 7) && (subclass == 0)) {
		return "Serial Controller";
		// serial bus controller
	} else if ((class == 0xc) && (subclass == 0x3) && (progif == 0x0)) {
		return "UHCI USB Controller";
	} else if ((class == 0xc) && (subclass == 0x3) && (progif == 0x10)) {
		return "OHCI USB Controller";
	} else if ((class == 0xc) && (subclass == 0x3) && (progif == 0x20)) {
		return "EHCI USB Controller";
	} else if ((class == 0xc) && (subclass == 0x3) && (progif == 0x30)) {
		return "xHCI USB Controller";
	} else if ((class == 0xc) && (subclass == 0x3) && (progif == 0xfe)) {
		return "USB Device";
	} else if ((class == 0xc) && (subclass == 0x5)) {
		return "SMBus Controller";
	} else {
		return "Unknown Class";
	}
}

void lspci_print_device(unsigned int bus, unsigned int device, unsigned int function,
						void* cfg_space, const char* drv_name) {
	struct PCIConfigHeader* cfg = cfg_space;
	printf("%x:%x.%x Vendor %x Device %x [%s]\n", bus, device, function, cfg->vendor,
		   cfg->device, pci_class_to_str(cfg->class, cfg->subclass, cfg->progif));
	if (verbose_level > 0) {
		if ((cfg->header_type & 0x7f) == 0) {
			for (int i = 0; i < 6; i++) {
				if (cfg->bar[i] && (cfg->bar[i] & 1)) {
					printf("BAR%d IO %x\n", i, (unsigned int)cfg->bar[i] & 0xFFFFFFFC);
				} else if (cfg->bar[i] && (cfg->bar[i] & 6)) {
					if (cfg->bar[i] & 8) {
						printf("BAR%d MEM64 %x hi %x pref\n", i,
							   (unsigned int)cfg->bar[i] & 0xFFFFFFF0,
							   (unsigned int)cfg->bar[i + 1]);
					} else {
						printf("BAR%d MEM64 %x hi %x\n", i,
							   (unsigned int)cfg->bar[i] & 0xFFFFFFF0,
							   (unsigned int)cfg->bar[i + 1]);
					}
				} else if (cfg->bar[i]) {
					if (cfg->bar[i] & 8) {
						printf("BAR%d MEM32 %x pref\n", i,
							   (unsigned int)cfg->bar[i] & 0xFFFFFFF0);
					} else {
						printf("BAR%d MEM32 %x\n", i,
							   (unsigned int)cfg->bar[i] & 0xFFFFFFF0);
					}
				}
			}
		} else {
			printf("PCI header type %d\n", cfg->header_type);
		}
		if (cfg->intr_pin) {
			printf("Interrupt: Pin %c , IRQ %d\n", cfg->intr_pin + 'A' - 1,
				   cfg->intr_line);
		}
		if (drv_name[0]) {
			printf("Driver: %s\n", drv_name);
		} else {
			puts("Driver: <none>");
		}
		putchar('\n');
	}
}

int main(int argc, char* argv[]) {
	if (argc > 1) {
		if (strcmp(argv[1], "-v") == 0) {
			verbose_level = 1;
		} else if (strcmp(argv[1], "-vv") == 0) {
			verbose_level = 2;
		}
	}
	void* cfg_space = malloc(4096);
	struct PciKcall pcikcall;
	pcikcall.op = PCI_KCALL_OP_FIRST_ADDR;
	if (kcall("pci", (unsigned int)&pcikcall)) {
		pcikcall.op = PCI_KCALL_OP_READ_CONFIG;
		pcikcall.ptr = cfg_space;
		kcall("pci", (unsigned int)&pcikcall);
		char drv_name[64];
		pcikcall.op = PCI_KCALL_OP_DRIVER_NAME;
		pcikcall.ptr = drv_name;
		kcall("pci", (unsigned int)&pcikcall);
		lspci_print_device(pcikcall.bus, pcikcall.device, pcikcall.function, cfg_space,
						   drv_name);
	}

	pcikcall.op = PCI_KCALL_OP_NEXT_ADDR;
	while (kcall("pci", (unsigned int)&pcikcall)) {
		pcikcall.op = PCI_KCALL_OP_READ_CONFIG;
		pcikcall.ptr = cfg_space;
		kcall("pci", (unsigned int)&pcikcall);
		char drv_name[64];
		pcikcall.op = PCI_KCALL_OP_DRIVER_NAME;
		pcikcall.ptr = drv_name;
		kcall("pci", (unsigned int)&pcikcall);
		lspci_print_device(pcikcall.bus, pcikcall.device, pcikcall.function, cfg_space,
						   drv_name);
		pcikcall.op = PCI_KCALL_OP_NEXT_ADDR;
	}
}
