/*
 * Power management
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
#include <defs.h>
#include <proc/kcall.h>

#ifndef __riscv
#include <common/x86.h>
#endif

#include "hal.h"

#define POWER_KCALL_OP_SHUTDOWN 0
#define POWER_KCALL_OP_REBOOT 1
#define POWER_KCALL_OP_SUSPEND 2
#define POWER_KCALL_OP_HIBERNATE 3

static int power_kcall_handler(unsigned int op) {
	switch (op) {
	case POWER_KCALL_OP_SHUTDOWN:
		hal_shutdown();
		break;
	case POWER_KCALL_OP_REBOOT:
		hal_reboot();
		break;
	case POWER_KCALL_OP_SUSPEND:
		return ERROR_INVAILD;
		break;
	case POWER_KCALL_OP_HIBERNATE:
		return ERROR_INVAILD;
		break;
	default:
		return ERROR_INVAILD;
	}
	return ERROR_INVAILD;
}

void hal_power_init(void) {
	kcall_set("power", power_kcall_handler);
}

void hal_shutdown(void) {
	cprintf("[hal] shutting down\n");
#ifndef __riscv
	outw(0x604, 0x2000); // QEMU
	outw(0xB004, 0x2000); // Bochs
	outw(0x4004, 0x3400); // VirtualBox
#endif
	panic("You can safely power off your computer");
}

void hal_reboot(void) {
	cprintf("[hal] reboot using port 0xcf9\n");
#ifndef __riscv
	outb(0xcf9, 6);
#endif
	panic("You can safely reboot your computer");
}
