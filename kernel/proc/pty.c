/*
 * Pseudoterminal
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
#include <common/spinlock.h>
#include <defs.h>

#include "pty.h"

struct PseudoTerminal pty[PTY_MAX];

void pty_init(void) {
	memset(pty, 0, sizeof(pty));
}

int pty_create(void) {
	int ptyid = -1;
	for (int i = 0; i < PTY_MAX; i++) {
		if (!pty[i].owner) {
			ptyid = i;
		}
	}
	if (ptyid < 0) {
		return ERROR_OUT_OF_SPACE;
	}

	pty[ptyid].owner = myproc();
	pty[ptyid].input_buffer = kalloc();
	pty[ptyid].output_buffer = kalloc();
	initlock(&pty[ptyid].lock, "pty");

	return ptyid;
}

int pty_close(int ptyid) {
	if (pty[ptyid].owner != myproc()) {
		return ERROR_NO_PERM;
	}
	kfree(pty[ptyid].input_buffer);
	kfree(pty[ptyid].output_buffer);
	pty[ptyid].owner = 0;
	return 0;
}

int pty_read(int ptyid, char* buf, int n) {
	acquire(&pty[ptyid].lock);
	while (pty[ptyid].input_begin == pty[ptyid].input_end) {
		sleep(&pty[ptyid].input_buffer, &pty[ptyid].lock);
	}
	for (int i = 0; i < n; i++) {
		buf[i] = pty[ptyid].input_buffer[pty[ptyid].input_end];
		pty[ptyid].input_end++;
		if (pty[ptyid].input_end == 4096) {
			pty[ptyid].input_end = 0;
		}
		if (pty[ptyid].input_begin == pty[ptyid].input_end) {
			release(&pty[ptyid].lock);
			return i + 1;
		}
	}
	release(&pty[ptyid].lock);
	return n;
}

int pty_write(int ptyid, const char* buf, int n) {
	acquire(&pty[ptyid].lock);
	for (int i = 0; i < n; i++) {
		pty[ptyid].output_buffer[pty[ptyid].output_begin] = buf[i];
		pty[ptyid].output_begin++;
		if (pty[ptyid].output_begin == 4096) {
			pty[ptyid].output_begin = 0;
		}
	}
	release(&pty[ptyid].lock);
	return n;
}

int pty_read_output(int ptyid, char* buf, int n) {
	if (pty[ptyid].owner != myproc()) {
		return ERROR_NO_PERM;
	}
	acquire(&pty[ptyid].lock);
	if (pty[ptyid].output_begin == pty[ptyid].output_end) {
		release(&pty[ptyid].lock);
		return 0;
	}
	for (int i = 0; i < n; i++) {
		buf[i] = pty[ptyid].output_buffer[pty[ptyid].output_end];
		pty[ptyid].output_end++;
		if (pty[ptyid].output_end == 4096) {
			pty[ptyid].output_end = 0;
		}
		if (pty[ptyid].output_begin == pty[ptyid].output_end) {
			release(&pty[ptyid].lock);
			return i + 1;
		}
	}
	release(&pty[ptyid].lock);
	return n;
}

int pty_write_input(int ptyid, const char* buf, int n) {
	if (pty[ptyid].owner != myproc()) {
		return ERROR_NO_PERM;
	}
	acquire(&pty[ptyid].lock);
	for (int i = 0; i < n; i++) {
		pty[ptyid].input_buffer[pty[ptyid].input_begin] = buf[i];
		pty[ptyid].input_begin++;
		if (pty[ptyid].input_begin == 4096) {
			pty[ptyid].input_begin = 0;
		}
	}
	wakeup(&pty[ptyid].input_buffer);
	release(&pty[ptyid].lock);
	return n;
}
