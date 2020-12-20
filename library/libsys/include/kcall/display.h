/*
 * Display Subsystem user mode API
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

#ifndef _LIBSYS_KCALL_DISPLAY_H
#define _LIBSYS_KCALL_DISPLAY_H

#include <panicos.h>

enum DisplayKCallOp {
	DISPLAY_KCALL_OP_ENABLE = 0,
	DISPLAY_KCALL_OP_DISABLE = 1,
	DISPLAY_KCALL_OP_FIND = 2,
	DISPLAY_KCALL_OP_GET_PREFERRED = 3,
	DISPLAY_KCALL_OP_UPDATE = 4,
	DISPLAY_KCALL_OP_GET_NAME = 5,
};

#define DISPLAY_KCALL_FLAG_NEED_UPDATE (1 << 0)

struct DisplayKcall {
	enum DisplayKCallOp op;
	char* str;
	unsigned int display_id;
	unsigned int xres;
	unsigned int yres;
	unsigned int flag;
	void* framebuffer;
};

static inline void* display_enable(unsigned int display_id, unsigned int xres,
								   unsigned int yres, unsigned int* flag) {
	struct DisplayKcall d = {
		.op = DISPLAY_KCALL_OP_ENABLE,
		.xres = xres,
		.yres = yres,
		.display_id = display_id,
	};
	if (kcall("display", (unsigned int)&d) < 0) {
		return 0;
	}
	*flag = d.flag;
	return d.framebuffer;
}

static inline void display_disable(unsigned int display_id) {
	struct DisplayKcall d = {
		.op = DISPLAY_KCALL_OP_DISABLE,
		.display_id = display_id,
	};
	kcall("display", (unsigned int)&d);
}

static inline int display_find(void) {
	struct DisplayKcall d = {
		.op = DISPLAY_KCALL_OP_FIND,
	};
	if (kcall("display", (unsigned int)&d) < 0) {
		return -1;
	}
	return d.display_id;
}

static inline int display_get_preferred(unsigned int display_id, unsigned int* xres,
										unsigned int* yres) {
	struct DisplayKcall d = {
		.op = DISPLAY_KCALL_OP_GET_PREFERRED,
		.display_id = display_id,
	};
	if (kcall("display", (unsigned int)&d) < 0) {
		return -1;
	}
	*xres = d.xres;
	*yres = d.yres;
	return 0;
}

static inline void display_update(unsigned int display_id) {
	struct DisplayKcall d = {
		.op = DISPLAY_KCALL_OP_UPDATE,
		.display_id = display_id,
	};
	kcall("display", (unsigned int)&d);
}

static inline int display_get_name(unsigned int display_id, char* name) {
	struct DisplayKcall d = {
		.op = DISPLAY_KCALL_OP_GET_NAME,
		.display_id = display_id,
		.str = name,
	};
	return kcall("display", (unsigned int)&d);
}

#endif
