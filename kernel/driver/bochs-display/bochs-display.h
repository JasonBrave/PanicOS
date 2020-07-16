#ifndef _DRIVER_BOCHS_DISPLAY_H
#define _DRIVER_BOCHS_DISPLAY_H

#include <common/types.h>
#include <driver/pci/pci.h>

struct BochsDisplayMMIO {
	uint16_t id;
	uint16_t xres;
	uint16_t yres;
	uint16_t bpp;
	uint16_t enable;
	uint16_t bank;
	uint16_t virt_width;
	uint16_t virt_height;
	uint16_t x_offset;
	uint16_t y_offset;
} PACKED;

extern struct BochsDisplay {
	struct PciAddress address;
	volatile struct BochsDisplayMMIO* mmio;
} bochs_display;

void bochs_display_init(void);
phyaddr_t bochs_display_enable(int xres, int yres);

#endif
