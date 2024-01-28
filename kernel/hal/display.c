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
	const struct FramebufferDriver *driver;
	const char *name;
	void *private;
	unsigned int preferred_xres, preferred_yres;
	unsigned int maximum_xres, maximum_yres;
} framebuffer_device[HAL_DISPLAY_MAX];

struct EDIDStdTimingInformation {
	uint8_t x_resolution;
	uint8_t aspect;
} PACKED;

struct EDIDDetailTiming {
	uint16_t pixel_clock;
	uint8_t horizontal_active;
	uint8_t horizontal_blanking;
	uint8_t horizontal_misc;
	uint8_t vertical_active;
	uint8_t vertical_blanking;
	uint8_t vertical_misc;
} PACKED;

static void *hal_display_modeswitch(struct FramebufferDevice *fbdev, int xres, int yres) {
	phyaddr_t fb = fbdev->driver->enable(fbdev->private, xres, yres);
#ifndef __riscv
	mappages(myproc()->pgdir, (void *)PROC_MMAP_BOTTOM, 16 * 1024 * 1024, fb, PTE_U | PTE_W);
#endif
	return (void *)PROC_MMAP_BOTTOM;
}

static int hal_display_kcall_handler(unsigned int display_struct) {
	enum DisplayKCallOp {
		DISPLAY_KCALL_OP_ENABLE = 0,
		DISPLAY_KCALL_OP_DISABLE = 1,
		DISPLAY_KCALL_OP_FIND = 2,
		DISPLAY_KCALL_OP_GET_PREFERRED = 3,
		DISPLAY_KCALL_OP_UPDATE = 4,
		DISPLAY_KCALL_OP_GET_NAME = 5,
	};

#define DISPLAY_KCALL_FLAG_NEED_UPDATE (1 << 0)
#define DISPLAY_ID_BOOT_FRAMEBUFFER 0xB005FB

	struct DisplayKcall {
		enum DisplayKCallOp op;
		char *str;
		unsigned int display_id;
		unsigned int xres;
		unsigned int yres;
		unsigned int flag;
		void *framebuffer;
	} *dc = (void *)display_struct;

	switch (dc->op) {
	case DISPLAY_KCALL_OP_ENABLE:
		if (dc->display_id == DISPLAY_ID_BOOT_FRAMEBUFFER
			&& boot_graphics_mode.mode == BOOT_GRAPHICS_MODE_FRAMEBUFFER) {
#ifndef __riscv
			mappages(myproc()->pgdir, (void *)PROC_MMAP_BOTTOM, 16 * 1024 * 1024,
					 boot_graphics_mode.fb_addr, PTE_U | PTE_W);
#endif
			dc->framebuffer = (void *)PROC_MMAP_BOTTOM;
			dc->flag = 0;
			return 0;
		} else if (framebuffer_device[dc->display_id].driver) {
			if (dc->xres > framebuffer_device[dc->display_id].maximum_xres
				|| dc->yres > framebuffer_device[dc->display_id].maximum_yres) {
				return ERROR_INVAILD;
			}
			dc->framebuffer
				= hal_display_modeswitch(&framebuffer_device[dc->display_id], dc->xres, dc->yres);
			dc->flag = 0;
			if (framebuffer_device[dc->display_id].driver->update) {
				dc->flag |= DISPLAY_KCALL_FLAG_NEED_UPDATE;
			}
			return 0;
		} else {
			return ERROR_NOT_EXIST;
		}
		break;
	case DISPLAY_KCALL_OP_DISABLE:
		if (dc->display_id == DISPLAY_ID_BOOT_FRAMEBUFFER
			&& boot_graphics_mode.mode == BOOT_GRAPHICS_MODE_FRAMEBUFFER) {
			cprintf("[display] boot-framebuffer disable unsupported");
			return ERROR_INVAILD;
		} else if (framebuffer_device[dc->display_id].driver) {
			framebuffer_device[dc->display_id].driver->disable(
				framebuffer_device[dc->display_id].private);
			return 0;
		}
		break;
	case DISPLAY_KCALL_OP_FIND:
		for (int i = 0; i < HAL_DISPLAY_MAX; i++) {
			if (framebuffer_device[i].driver) {
				dc->display_id = i;
				return 0;
			}
		}
		if (boot_graphics_mode.mode == BOOT_GRAPHICS_MODE_FRAMEBUFFER) {
			dc->display_id = DISPLAY_ID_BOOT_FRAMEBUFFER;
			return 0;
		}
		return ERROR_NOT_EXIST;
		break;
	case DISPLAY_KCALL_OP_GET_PREFERRED:
		if (dc->display_id == DISPLAY_ID_BOOT_FRAMEBUFFER
			&& boot_graphics_mode.mode == BOOT_GRAPHICS_MODE_FRAMEBUFFER) {
			dc->xres = boot_graphics_mode.width;
			dc->yres = boot_graphics_mode.height;
			return 0;
		} else if (framebuffer_device[dc->display_id].driver) {
			dc->xres = framebuffer_device[dc->display_id].preferred_xres;
			dc->yres = framebuffer_device[dc->display_id].preferred_yres;
			return 0;
		} else {
			return ERROR_NOT_EXIST;
		}
		break;
	case DISPLAY_KCALL_OP_UPDATE:
		if (dc->display_id == DISPLAY_ID_BOOT_FRAMEBUFFER
			&& boot_graphics_mode.mode == BOOT_GRAPHICS_MODE_FRAMEBUFFER) {
			cprintf("[display] boot-framebuffer update unsupported");
			return ERROR_INVAILD;
		} else if (framebuffer_device[dc->display_id].driver
				   && framebuffer_device[dc->display_id].driver->update) {
			framebuffer_device[dc->display_id].driver->update(
				framebuffer_device[dc->display_id].private);
			return 0;
		} else {
			return ERROR_NOT_EXIST;
		}
		break;
	case DISPLAY_KCALL_OP_GET_NAME:
		if (dc->display_id == DISPLAY_ID_BOOT_FRAMEBUFFER
			&& boot_graphics_mode.mode == BOOT_GRAPHICS_MODE_FRAMEBUFFER) {
			strncpy(dc->str, "boot-framebuffer", 64);
			return 0;
		} else if (framebuffer_device[dc->display_id].driver
				   && framebuffer_device[dc->display_id].name) {
			strncpy(dc->str, framebuffer_device[dc->display_id].name, 64);
			return 0;
		} else {
			return ERROR_NOT_EXIST;
		}
	}
	return ERROR_INVAILD;
}

static struct FramebufferDevice *hal_display_alloc_dev(unsigned int *n) {
	for (int i = 0; i < HAL_DISPLAY_MAX; i++) {
		if (!framebuffer_device[i].driver) {
			*n = i;
			return &framebuffer_device[i];
		}
	}
	return 0;
}

void hal_display_register_device(const char *name, void *private,
								 const struct FramebufferDriver *driver) {
	unsigned int devid;
	struct FramebufferDevice *dev = hal_display_alloc_dev(&devid);
	if (!dev) {
		panic("too many display device");
	}
	cprintf("[hal] Display device %d %s update%s edid%s\n", devid, name,
			BOOL2SIGN((int)driver->update), BOOL2SIGN((int)driver->read_edid));
	dev->driver = driver;
	dev->name = name;
	dev->private = private;
	dev->preferred_xres = 1024;
	dev->preferred_yres = 768;
	dev->maximum_xres = 1920;
	dev->maximum_yres = 1080;
	if (!driver->read_edid) {
		return;
	}
	uint8_t edid[128];
	if (driver->read_edid(private, edid, 128) != 128) {
		return;
	}
	if (*(uint64_t *)edid != 0x00ffffffffffff00) {
		cprintf("[hal] invaild edid header\n");
		return;
	}
	uint16_t pnpid = (edid[8] << 8) | edid[9];
	char vendor[4];
	vendor[0] = ((pnpid >> 10) & 0x1f) + 'A' - 1;
	vendor[1] = ((pnpid >> 5) & 0x1f) + 'A' - 1;
	vendor[2] = ((pnpid >> 0) & 0x1f) + 'A' - 1;
	vendor[3] = '\0';
	if (edid[20] & (1 << 7)) {
		const char *edid_video_inf[] = {
			[0x0] = "undefined", [0x2] = "HDMIa",		[0x3] = "HDMIb",
			[0x4] = "MDDI",		 [0x5] = "DisplayPort",
		};
		cprintf("[hal] Monitor %s digital %s edid ver %d.%d\n", vendor,
				edid_video_inf[edid[20] & 0xf], edid[18], edid[19]);
	} else {
		cprintf("[hal] Monitor %s analog edid ver %d.%d\n", vendor, edid[18], edid[19]);
	}
	// EDID Standard Timing Information
	for (int i = 0; i < 8; i++) {
		struct EDIDStdTimingInformation *stdtiming = (void *)edid + 38 + i * 2;
		if (!stdtiming->x_resolution) {
			continue;
		}
		if ((stdtiming->x_resolution == 1) && (stdtiming->aspect == 1)) {
			continue;
		}
		const struct {
			int x, y;
		} edid_std_aspect[4] = {[0] = {16, 10}, [1] = {4, 3}, [2] = {5, 4}, [3] = {16, 9}};
		unsigned int x = (stdtiming->x_resolution + 31) * 8;
		unsigned int y = (stdtiming->x_resolution + 31) * 8
						 / edid_std_aspect[(stdtiming->aspect >> 6) & 3].x
						 * edid_std_aspect[(stdtiming->aspect >> 6) & 3].y;
		if (x >= dev->maximum_xres && y >= dev->maximum_yres) {
			dev->maximum_xres = x;
			dev->maximum_yres = y;
		}
	}
	// EDID Detailed Timing Infomation
	for (int i = 0; i < 4; i++) {
		struct EDIDDetailTiming *detailtiming = (void *)edid + 54 + i * 18;
		if (detailtiming->pixel_clock) {
			unsigned int x
				= detailtiming->horizontal_active | (detailtiming->horizontal_misc >> 4 << 8);
			unsigned int y
				= detailtiming->vertical_active | (detailtiming->vertical_misc >> 4 << 8);
			if (x >= dev->preferred_xres && y >= dev->preferred_yres) {
				dev->preferred_xres = x;
				dev->preferred_yres = y;
			}
		}
	}
	if (dev->preferred_xres >= dev->maximum_xres && dev->preferred_yres >= dev->maximum_yres) {
		dev->maximum_xres = dev->preferred_xres;
		dev->maximum_yres = dev->preferred_yres;
	}
	cprintf("[hal] Monitor display xres %d yres %d max xres %d yres %d\n", dev->preferred_xres,
			dev->preferred_yres, dev->maximum_xres, dev->maximum_yres);
}

void hal_display_init(void) {
	memset(framebuffer_device, 0, sizeof(framebuffer_device));
	kcall_set("display", hal_display_kcall_handler);
}
