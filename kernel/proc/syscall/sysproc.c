/*
 * Process management system calls
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
#include <common/x86.h>
#include <core/mmu.h>
#include <core/proc.h>
#include <defs.h>
#include <memlayout.h>
#include <param.h>

int sys_fork(void) {
	return fork();
}

int sys_exit(void) {
	exit();
	return 0; // not reached
}

int sys_wait(void) {
	return wait();
}

int sys_kill(void) {
	int pid;

	if (argint(0, &pid) < 0)
		return -1;
	return kill(pid);
}

int sys_getpid(void) {
	return myproc()->pid;
}

int sys_sbrk(void) {
	int addr;
	int n;

	if (argint(0, &n) < 0)
		return -1;
	addr = PROC_HEAP_BOTTOM + myproc()->heap_size;
	if (growproc(n) < 0)
		return -1;
	return addr;
}

int sys_sleep(void) {
	int n;
	unsigned int ticks0;

	if (argint(0, &n) < 0)
		return -1;
	acquire(&tickslock);
	ticks0 = ticks;
	while (ticks - ticks0 < n) {
		if (myproc()->killed) {
			release(&tickslock);
			return -1;
		}
		sleep(&ticks, &tickslock);
	}
	release(&tickslock);
	return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void) {
	unsigned int xticks;

	acquire(&tickslock);
	xticks = ticks;
	release(&tickslock);
	return xticks;
}

int sys_chdir(void) {
	char* dir;
	if (argstr(0, &dir) < 0) {
		return -1;
	}

	if (dir[0] == '/') { // absolute path
		int mode = vfs_file_get_mode(dir);
		if (mode < 0) {
			return mode;
		}
		if (mode & 0040000) { // is a directory
			struct proc* p = myproc();
			p->cwd.parts = vfs_path_split(dir, p->cwd.pathbuf);
		} else { // not a directory
			return ERROR_NOT_DIRECTORY;
		}
		return 0;
	} else { // relative path
		struct VfsPath newpath = {.pathbuf = kalloc()};
		newpath.parts = vfs_path_split(dir, newpath.pathbuf);
		vfs_get_absolute_path(&newpath);
		char fullpath[64];
		vfs_path_tostring(newpath, fullpath);
		int mode = vfs_file_get_mode(fullpath);
		if (mode < 0) {
			kfree(newpath.pathbuf);
			return mode;
		}
		if (mode & 0040000) { // is a directory
			struct proc* p = myproc();
			kfree(p->cwd.pathbuf);
			p->cwd = newpath;
		} else { // not a directory
			kfree(newpath.pathbuf);
			return ERROR_NOT_DIRECTORY;
		}
		return 0;
	}
}

int sys_getcwd(void) {
	char* dir;
	if (argstr(0, &dir) < 0) {
		return -1;
	}
	vfs_path_tostring(myproc()->cwd, dir);
	return 0;
}

int sys_dynamic_load(void) {
	char* name;
	unsigned int* dynamic;
	unsigned int* entry;
	if (argstr(0, &name) < 0 || argptr(1, (char**)&dynamic, sizeof(unsigned int)) ||
		argptr(2, (char**)&entry, sizeof(unsigned int))) {
		return -1;
	}
	return proc_load_dynamic(myproc(), name, dynamic, entry);
}
