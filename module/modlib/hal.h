/*
 * Kernel module HAL helper functions
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

#ifndef _MODLIB_HAL_H
#define _MODLIB_HAL_H

#include <kernsrv.h>

struct BlockDeviceDriver {
	int (*block_read)(void* private, unsigned int begin, int count, void* buf);
	int (*block_write)(void* private, unsigned int begin, int count, const void* buf);
};

struct FramebufferDriver {
	phyaddr_t (*enable)(void* private, int xres, int yres);
	void (*disable)(void* private);
	void (*update)(void* private);
	unsigned int (*read_edid)(void* private, void* buffer, unsigned int bytes);
};

static inline void hal_block_register_device(const char* name, void* private,
											 const struct BlockDeviceDriver* driver) {
	return kernsrv->hal_block_register_device(name, private, driver);
}

static inline void hal_display_register_device(const char* name, void* private,
											   const struct FramebufferDriver* driver) {
	return kernsrv->hal_display_register_device(name, private, driver);
}

static inline void hal_mouse_update(unsigned int data) {
	return kernsrv->hal_mouse_update(data);
}

static inline void hal_keyboard_update(unsigned int data) {
	return kernsrv->hal_keyboard_update(data);
}

#endif
