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
#include <param.h>
#include <proc/kcall.h>

#include "hal.h"

#define HAL_DISPLAY_MAX 8

struct FramebufferDevice {
	struct FramebufferDriver* driver;
	void* private;
} framebuffer_device[HAL_DISPLAY_MAX];

static void* hal_display_modeswitch(struct FramebufferDevice* fbdev, int xres,
									int yres) {
	phyaddr_t fb = fbdev->driver->enable(fbdev->private, xres, yres);
	mappages(myproc()->pgdir, (void*)PROC_MMAP_BOTTOM, 16 * 1024 * 1024, fb,
			 PTE_U | PTE_W);
	return (void*)PROC_MMAP_BOTTOM;
}

static int hal_display_kcall_handler(unsigned int display_struct) {
	struct {
		int xres;
		int yres;
		void* framebuffer;
	}* dc = (void*)display_struct;

	for (int i = 0; i < HAL_DISPLAY_MAX; i++) {
		if (framebuffer_device[i].driver) {
			dc->framebuffer =
				hal_display_modeswitch(&framebuffer_device[i], dc->xres, dc->yres);
			return 0;
		}
	}
	return ERROR_NOT_EXIST;
}

void hal_display_register_device(const char* name, void* private,
								 struct FramebufferDriver* driver) {
	struct FramebufferDevice* dev = 0;
	for (int i = 0; i < HAL_DISPLAY_MAX; i++) {
		if (!framebuffer_device[i].driver) {
			dev = &framebuffer_device[i];
		}
	}
	if (!dev) {
		panic("too many display device");
	}

	cprintf("[hal] Display device %s added\n", name);
	dev->driver = driver;
	dev->private = private;
}

void hal_display_init(void) {
	memset(framebuffer_device, 0, sizeof(framebuffer_device));
	kcall_set("display", hal_display_kcall_handler);
}
