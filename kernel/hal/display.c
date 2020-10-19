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
	const struct FramebufferDriver* driver;
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
#define DISPLAY_OP_ENABLE 0
#define DISPLAY_OP_UPDATE 1
#define DISPLAY_OP_FIND 2

#define DISPLAY_FLAG_NEED_UPDATE (1 << 0)

	struct {
		int op;
		int display_id;
		int xres;
		int yres;
		int flag;
		void* framebuffer;
	}* dc = (void*)display_struct;

	if (dc->op == DISPLAY_OP_ENABLE) {
		if (framebuffer_device[dc->display_id].driver) {
			dc->framebuffer = hal_display_modeswitch(
				&framebuffer_device[dc->display_id], dc->xres, dc->yres);
			dc->flag = 0;
			if (framebuffer_device[dc->display_id].driver->update) {
				dc->flag |= DISPLAY_FLAG_NEED_UPDATE;
			}
			return 0;
		} else {
			return ERROR_NOT_EXIST;
		}
	} else if (dc->op == DISPLAY_OP_UPDATE) {
		if (framebuffer_device[dc->display_id].driver &&
			framebuffer_device[dc->display_id].driver->update) {
			framebuffer_device[dc->display_id].driver->update(
				framebuffer_device[dc->display_id].private);
			return 0;
		} else {
			return ERROR_NOT_EXIST;
		}
	} else if (dc->op == DISPLAY_OP_FIND) {
		for (int i = 0; i < HAL_DISPLAY_MAX; i++) {
			if (framebuffer_device[i].driver) {
				dc->display_id = i;
				return 0;
			}
		}
		dc->display_id = -1;
		return ERROR_NOT_EXIST;
	}
	return ERROR_INVAILD;
}

static struct FramebufferDevice* hal_display_alloc_dev(unsigned int* n) {
	for (int i = 0; i < HAL_DISPLAY_MAX; i++) {
		if (!framebuffer_device[i].driver) {
			*n = i;
			return &framebuffer_device[i];
		}
	}
	return 0;
}

void hal_display_register_device(const char* name, void* private,
								 const struct FramebufferDriver* driver) {
	unsigned int devid;
	struct FramebufferDevice* dev = hal_display_alloc_dev(&devid);
	if (!dev) {
		panic("too many display device");
	}
	cprintf("[hal] Display device %d %s update%s edid%s\n", devid, name,
			BOOL2SIGN((int)driver->update), BOOL2SIGN((int)driver->read_edid));
	dev->driver = driver;
	dev->private = private;
	if (!driver->read_edid) {
		return;
	}
	uint8_t edid[128];
	if (driver->read_edid(private, edid, 128) != 128) {
		return;
	}
	if (*(uint64_t*)edid != 0x00ffffffffffff00) {
		cprintf("[hal] invaild edid header");
		return;
	}
	uint16_t pnpid = (edid[8] << 8) | edid[9];
	char vendor[4];
	vendor[0] = ((pnpid >> 10) & 0x1f) + 'A' - 1;
	vendor[1] = ((pnpid >> 5) & 0x1f) + 'A' - 1;
	vendor[2] = ((pnpid >> 0) & 0x1f) + 'A' - 1;
	vendor[3] = '\0';
	if (edid[20] & (1 << 7)) {
		const char* edid_video_inf[] = {
			[0x0] = "undefined", [0x2] = "HDMIa",		[0x3] = "HDMIb",
			[0x4] = "MDDI",		 [0x5] = "DisplayPort",
		};
		cprintf("[hal] Monitor %s digital %s edid ver %d.%d\n", vendor,
				edid_video_inf[edid[20] & 0xf], edid[18], edid[19]);
	} else {
		cprintf("[hal] Monitor %s analog edid ver %d.%d\n", vendor, edid[18], edid[19]);
	}
}

void hal_display_init(void) {
	memset(framebuffer_device, 0, sizeof(framebuffer_device));
	kcall_set("display", hal_display_kcall_handler);
}
