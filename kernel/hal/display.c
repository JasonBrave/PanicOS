/*
 * display device hardware abstraction layer
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

#include <common/errorcode.h>
#include <core/proc.h>
#include <defs.h>
#include <driver/bochs-display/bochs-display.h>
#include <param.h>
#include <proc/kcall.h>

struct DisplayControl {
	int xres;
	int yres;
	void* framebuffer;
};

void* hal_display_modeswitch(int xres, int yres) {
	phyaddr_t fb = bochs_display_enable(xres, yres);
	mappages(myproc()->pgdir, (void*)PROC_MMAP_BOTTOM, 16 * 1024 * 1024, fb,
			 PTE_U | PTE_W);
	return (void*)PROC_MMAP_BOTTOM;
}

int hal_display_kcall_handler(unsigned int display_struct) {
	struct DisplayControl* dc = (void*)display_struct;
	if (!bochs_display.mmio) {
		return ERROR_NOT_EXIST;
	}
	dc->framebuffer = hal_display_modeswitch(dc->xres, dc->yres);
	return 0;
}

void hal_display_init(void) {
	bochs_display_init();
	if (bochs_display.mmio) {
		kcall_set("display", hal_display_kcall_handler);
	}
}
