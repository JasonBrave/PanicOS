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

#include <kcall/pci.h>
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

static unsigned int powof2(unsigned int power) {
	unsigned int n = 1;
	for (unsigned int i = 0; i < power; i++) {
		n *= 2;
	}
	return n;
}

static void lspci_print_type0_header(const void* cfg_space, const struct PciKcallResource* pcires) {
	const struct PCIType0ConfigHeader* cfg = cfg_space;
	if (verbose_level > 1) {
		printf("Subsystem Vendor %x Device %x\n", cfg->subsys_vendor, cfg->subsys_device);
	}
	for (int i = 0; i < 6; i++) {
		if (cfg->bar[i] && (cfg->bar[i] & 1)) {
			printf("BAR%d IO %x - %lx\n", i, (unsigned int)cfg->bar[i] & 0xFFFFFFFC,
				   ((unsigned int)cfg->bar[i] & 0xFFFFFFFC) + (uint32_t)pcires->bar_size[i] - 1);
		} else if (cfg->bar[i] && (cfg->bar[i] & 6)) {
			uint64_t bar_val = ((uint64_t)cfg->bar[i + 1] << 32) | (cfg->bar[i] & 0xFFFFFFF0);
			if (cfg->bar[i] & 8) {
				printf("BAR%d MEM64 %llx - %llx pref\n", i, bar_val,
					   bar_val + pcires->bar_size[i] - 1);
			} else {
				printf("BAR%d MEM64 %llx - %llx\n", i, bar_val, bar_val + pcires->bar_size[i] - 1);
			}
		} else if (cfg->bar[i]) {
			uint32_t bar_val = (unsigned int)cfg->bar[i] & 0xFFFFFFF0;
			if (cfg->bar[i] & 8) {
				printf("BAR%d MEM32 %lx - %lx pref\n", i, bar_val,
					   bar_val + (uint32_t)pcires->bar_size[i] - 1);
			} else {
				printf("BAR%d MEM32 %lx - %lx\n", i, bar_val,
					   bar_val + (uint32_t)pcires->bar_size[i] - 1);
			}
		}
	}
	if (cfg->rombar) {
		printf("OpROM %lx - %lx\n", cfg->rombar & 0xfffff800,
			   (cfg->rombar & 0xfffff800) + (uint32_t)pcires->rombar_size - 1);
	}
}

static void lspci_print_type1_header(const void* cfg_space, const struct PciKcallResource* pcires) {
	const struct PCIType1ConfigHeader* cfg = cfg_space;
	printf("Bridge Primary %x Secondary %x Subordinate %x\n", cfg->pribus, cfg->secbus,
		   cfg->subbus);
	for (int i = 0; i < 2; i++) {
		if (cfg->bar[i] && (cfg->bar[i] & 1)) {
			printf("BAR%d IO %x - %lx\n", i, (unsigned int)cfg->bar[i] & 0xFFFFFFFC,
				   ((unsigned int)cfg->bar[i] & 0xFFFFFFFC) + (uint32_t)pcires->bar_size[i] - 1);
		} else if (cfg->bar[i] && (cfg->bar[i] & 6)) {
			uint64_t bar_val = ((uint64_t)cfg->bar[i + 1] << 32) | (cfg->bar[i] & 0xFFFFFFF0);
			if (cfg->bar[i] & 8) {
				printf("BAR%d MEM64 %llx - %llx pref\n", i, bar_val,
					   bar_val + pcires->bar_size[i] - 1);
			} else {
				printf("BAR%d MEM64 %llx - %llx\n", i, bar_val, bar_val + pcires->bar_size[i] - 1);
			}
		} else if (cfg->bar[i]) {
			uint32_t bar_val = (unsigned int)cfg->bar[i] & 0xFFFFFFF0;
			if (cfg->bar[i] & 8) {
				printf("BAR%d MEM32 %lx - %lx pref\n", i, bar_val,
					   bar_val + (uint32_t)pcires->bar_size[i] - 1);
			} else {
				printf("BAR%d MEM32 %lx - %lx\n", i, bar_val,
					   bar_val + (uint32_t)pcires->bar_size[i] - 1);
			}
		}
	}
	printf("IO window %x - %x\n",
		   (cfg->io_base & 1) ? (((cfg->io_base & 0xf0) << 8) | (cfg->io_base_upper << 16))
							  : ((cfg->io_base & 0xf0) << 8),
		   (cfg->io_limit & 1) ? (((cfg->io_limit & 0xf0) << 8) | (cfg->io_limit_upper << 16))
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
						const void* cfg_space, const char* drv_name,
						const struct PciKcallResource* pcires) {

	const struct PCIGenericConfigHeader* cfg = cfg_space;
	const char* device_name = pci_device_id_to_str(cfg->vendor, cfg->device);
	if (device_name) {
		printf("%x:%x.%x %s %x:%x [%s]\n", bus, device, function, device_name, cfg->vendor,
			   cfg->device, pci_class_to_str(cfg->pclass, cfg->subclass, cfg->progif));
	} else {
		printf("%x:%x.%x Vendor %x Device %x [%s]\n", bus, device, function, cfg->vendor,
			   cfg->device, pci_class_to_str(cfg->pclass, cfg->subclass, cfg->progif));
	}

	if (verbose_level > 0) {
		if (cfg->header_type == 0 || cfg->header_type == 0x80) {
			lspci_print_type0_header(cfg_space, pcires);
		} else if (cfg->header_type == 1 || cfg->header_type == 0x81) {
			lspci_print_type1_header(cfg_space, pcires);
		} else {
			printf("PCI header type %d\n", cfg->header_type);
		}
		if (cfg->intr_pin) {
			printf("Interrupt: Pin %c , IRQ %d\n", cfg->intr_pin + 'A' - 1, cfg->intr_line);
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
				} else if (*((uint8_t*)cfg + cap_off) == 0x5) {
					printf("[%x] Message Signaled Interrupts\n", cap_off);
					if (verbose_level > 2) {
						uint16_t msgctl = *(uint16_t*)(cfg_space + cap_off + 2);
						printf("    Message Control Enable%s, %d/%d Messages, "
							   "64Bits%s, Per-vector masking%s\n",
							   (msgctl & 1) ? "+" : "-", powof2((msgctl >> 1) & 7),
							   powof2((msgctl >> 4) & 7), (msgctl & (1 << 7)) ? "+" : "-",
							   (msgctl & (1 << 8)) ? "+" : "-");
						if ((msgctl & 1) && (msgctl & (1 << 7))) {
							printf("    Message address %lx, Message data %x\n",
								   *(uint32_t*)(cfg_space + cap_off + 4),
								   *(uint16_t*)(cfg_space + cap_off + 12));
						} else if (msgctl & 1) {
							printf("    Message address %lx, Message data %x\n",
								   *(uint32_t*)(cfg_space + cap_off + 4),
								   *(uint16_t*)(cfg_space + cap_off + 8));
						}
					}
				} else if (*((uint8_t*)cfg + cap_off) == 0x11) {
					printf("[%x] MSI-X\n", cap_off);
					if (verbose_level > 2) {
						uint16_t msgctl = *(uint16_t*)(cfg_space + cap_off + 2);
						printf("    Message Control Enable%s, FuncMask%s, Table Size %d\n",
							   (msgctl & (1 << 15)) ? "+" : "-", (msgctl & (1 << 14)) ? "+" : "-",
							   (msgctl & 0x7ff) + 1);
						uint32_t tableoff = *(uint32_t*)(cfg_space + cap_off + 4);
						printf("    Table offset %lx BAR %ld\n", tableoff & 0xfffffff8,
							   tableoff & 7);
						uint32_t pbaoff = *(uint32_t*)(cfg_space + cap_off + 8);
						printf("    PBA offset %lx BAR %ld\n", pbaoff & 0xfffffff8, pbaoff & 7);
					}
				} else {
					printf("[%x] %s\n", cap_off, pci_cap_to_str(*((uint8_t*)cfg + cap_off)));
				}
				cap_off = *((uint8_t*)cfg + cap_off + 1);
			}
			if (pciexpress && *(uint32_t*)((void*)cfg + 0x100)) {
				cap_off = 0x100;
				while (cap_off) {
					printf("[%x v%d] %s\n", cap_off, *(uint16_t*)((void*)cfg + cap_off + 2) & 0xf,
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
		} else if (strcmp(argv[1], "-vvv") == 0) {
			verbose_level = 3;
		}
	}
	void* cfg_space = malloc(4096);
	struct PCIAddr pciaddr;
	unsigned int pciid = pci_get_first_addr(&pciaddr);
	if (pciid == 0xffffffff) {
		return 0;
	}
	pci_read_config(pciid, cfg_space);
	char drv_name[64];
	pci_get_driver_name(pciid, drv_name);
	struct PciKcallResource pcires;
	pci_get_resource(pciid, &pcires);
	lspci_print_device(pciaddr.bus, pciaddr.device, pciaddr.function, cfg_space, drv_name, &pcires);

	pciid = pci_get_next_addr(pciid, &pciaddr);
	while (pciid) {
		pci_read_config(pciid, cfg_space);
		pci_get_driver_name(pciid, drv_name);
		pci_get_resource(pciid, &pcires);
		lspci_print_device(pciaddr.bus, pciaddr.device, pciaddr.function, cfg_space, drv_name,
						   &pcires);
		pciid = pci_get_next_addr(pciid, &pciaddr);
	}
}
