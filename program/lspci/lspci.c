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

const char* pci_class_to_str(uint8_t class, uint8_t subclass, uint8_t progif);
const char* pci_cap_to_str(unsigned int cap_id);
const char* pcie_ecap_to_str(unsigned int ecap_id);
const char* pci_device_id_to_str(uint16_t vendor, uint16_t device);

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

struct PCIGenericConfigHeader {
	uint16_t vendor, device;
	uint16_t command, status;
	uint8_t revision, progif, subclass, class;
	uint8_t cache_line, lat_timer, header_type, bist;
	uint8_t placeholder[36];
	uint8_t capptr, res[7];
	uint8_t intr_line, intr_pin;
	uint16_t shared;
} __attribute__((packed));

struct PCIType0ConfigHeader {
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

struct PCIType1ConfigHeader {
	uint16_t vendor, device;
	uint16_t command, status;
	uint8_t revision, progif, subclass, class;
	uint8_t cache_line, lat_timer, header_type, bist;
	uint32_t bar[2];
	uint8_t pribus, secbus, subbus, seclatmr;
	uint8_t io_base, io_limit;
	uint16_t sec_status;
	uint16_t mem_base, mem_limit;
	uint16_t pref_mem_base, pref_mem_limit;
	uint32_t pref_mem_base_upper, pref_mem_limit_upper;
	uint16_t io_base_upper, io_limit_upper;
	uint8_t capptr, res[7];
	uint8_t intr_line, intr_pin;
	uint16_t bridge_control;
} __attribute__((packed));

static void lspci_print_type0_header(void* cfg_space) {
	struct PCIType0ConfigHeader* cfg = cfg_space;
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
				printf("BAR%d MEM32 %x\n", i, (unsigned int)cfg->bar[i] & 0xFFFFFFF0);
			}
		}
	}
}

static void lspci_print_type1_header(void* cfg_space) {
	struct PCIType1ConfigHeader* cfg = cfg_space;
	printf("Bridge Primary %x Secondary %x Subordinate %x\n", cfg->pribus, cfg->secbus,
		   cfg->subbus);
	for (int i = 0; i < 2; i++) {
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
				printf("BAR%d MEM32 %x\n", i, (unsigned int)cfg->bar[i] & 0xFFFFFFF0);
			}
		}
	}
	printf("IO window %x - %x\n",
		   (cfg->io_base & 1)
			   ? (((cfg->io_base & 0xf0) << 8) | (cfg->io_base_upper << 16))
			   : ((cfg->io_base & 0xf0) << 8),
		   (cfg->io_limit & 1)
			   ? (((cfg->io_limit & 0xf0) << 8) | (cfg->io_limit_upper << 16))
			   : ((cfg->io_limit & 0xf0) << 8));
	printf("Memory Window %x - %x\n", (cfg->mem_base & 0xfff0) << 16,
		   (cfg->mem_limit & 0xfff0) << 16);
	printf("Prefetch Memory Window %x", (cfg->pref_mem_base & 0xfff0) << 16);
	if (cfg->pref_mem_base & 1) {
		printf(" hi %x", (unsigned int)cfg->pref_mem_base_upper);
	}
	printf(" - %x", (cfg->pref_mem_limit & 0xfff0) << 16);
	if (cfg->pref_mem_limit & 1) {
		printf(" hi %x", (unsigned int)cfg->pref_mem_limit_upper);
	}
	putchar('\n');
}

void lspci_print_device(unsigned int bus, unsigned int device, unsigned int function,
						void* cfg_space, const char* drv_name) {

	struct PCIGenericConfigHeader* cfg = cfg_space;
	const char* device_name = pci_device_id_to_str(cfg->vendor, cfg->device);
	if (device_name) {
		printf("%x:%x.%x %s %x:%x [%s]\n", bus, device, function, device_name,
			   cfg->vendor, cfg->device,
			   pci_class_to_str(cfg->class, cfg->subclass, cfg->progif));
	} else {
		printf("%x:%x.%x Vendor %x Device %x [%s]\n", bus, device, function,
			   cfg->vendor, cfg->device,
			   pci_class_to_str(cfg->class, cfg->subclass, cfg->progif));
	}

	if (verbose_level > 0) {
		if (cfg->header_type == 0 || cfg->header_type == 0x80) {
			lspci_print_type0_header(cfg_space);
		} else if (cfg->header_type == 1 || cfg->header_type == 0x81) {
			lspci_print_type1_header(cfg_space);
		} else {
			printf("PCI header type %d\n", cfg->header_type);
		}
		if (cfg->intr_pin) {
			printf("Interrupt: Pin %c , IRQ %d\n", cfg->intr_pin + 'A' - 1,
				   cfg->intr_line);
		}
		if (verbose_level > 1) {
			unsigned int cap_off = cfg->capptr;
			int pciexpress = 0;
			while (cap_off) {
				if (*((uint8_t*)cfg + cap_off) == 0x10) {
					pciexpress = 1;
					const char* pcie_cap_names[] = {
						[0x0] = "Endpoint",
						[0x1] = "Legacy Endpoint",
						[0x4] = "Root Port",
						[0x5] = "Switch Upstream Port",
						[0x6] = "Switch Downstream Port",
						[0x7] = "PCIe to PCI/PCI-X Bridge",
						[0x8] = "PCI/PCI-X to PCIe Bridge",
						[0x9] = "Root Complex Integrated Endpoint",
						[0xa] = "Root Complex Event Collector",

					};
					printf("[%x] PCI Express(v%d) %s\n", cap_off,
						   *((uint8_t*)cfg + cap_off + 2) & 0xf,
						   pcie_cap_names[(*((uint8_t*)cfg + cap_off + 2) >> 4) & 0xf]);
				} else {
					printf("[%x] %s\n", cap_off,
						   pci_cap_to_str(*((uint8_t*)cfg + cap_off)));
				}
				cap_off = *((uint8_t*)cfg + cap_off + 1);
			}
			if (pciexpress && *(uint32_t*)((void*)cfg + 0x100)) {
				cap_off = 0x100;
				while (cap_off) {
					printf("[%x v%d] %s\n", cap_off,
						   *(uint16_t*)((void*)cfg + cap_off + 2) & 0xf,
						   pcie_ecap_to_str(*(uint16_t*)((void*)cfg + cap_off)));
					cap_off = *(uint16_t*)((void*)cfg + cap_off + 2) >> 4;
				}
			}
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
