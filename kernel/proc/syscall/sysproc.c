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
#include <core/proc.h>
#include <defs.h>
#include <memlayout.h>
#include <param.h>
#include <proc/kcall.h>
#include <proc/pty.h>

int sys_fork(void) {
	return fork();
}

int sys_exit(void) {
	int status;
	argint(0, &status);
	exit(status);
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

int sys_kcall(void) {
	char* name;
	unsigned int arg;
	if (argstr(0, &name) < 0 || argint(1, (int*)&arg) < 0) {
		return -1;
	}
	return kcall(name, arg);
}

int sys_message_send(void) {
	int pid, size;
	void* data;
	if ((argint(0, &pid) < 0) || (argint(1, &size) < 0) || (argptr(2, (char**)&data, size) < 0)) {
		return -1;
	}
	struct proc* destproc = proc_search_pid(pid);
	if (!destproc) {
		return -1;
	}
	acquire(&destproc->msgqueue.lock);
	struct Message* destmsg = &destproc->msgqueue.queue[destproc->msgqueue.begin];
	destmsg->pid = myproc()->pid;
	destmsg->size = size;
	destmsg->addr = pgalloc(PGROUNDUP(size) / 4096);
	memmove(destmsg->addr, data, size);
	destproc->msgqueue.begin++;
	if (destproc->msgqueue.begin == MESSAGE_MAX) {
		destproc->msgqueue.begin = 0;
	}
	wakeup(&destproc->msgqueue);
	release(&destproc->msgqueue.lock);
	return 0;
}

int sys_message_receive(void) {
	void* data;
	if (argptr(0, (char**)&data, 4 * 1024 * 1024) < 0) {
		return -1;
	}
	acquire(&myproc()->msgqueue.lock);
	if (myproc()->msgqueue.begin == myproc()->msgqueue.end) {
		release(&myproc()->msgqueue.lock);
		return 0;
	}
	struct Message* thismsg = &myproc()->msgqueue.queue[myproc()->msgqueue.end];
	int ret = thismsg->pid;
	memmove(data, thismsg->addr, thismsg->size);
	pgfree(thismsg->addr, PGROUNDUP(thismsg->size) / 4096);
	myproc()->msgqueue.end++;
	if (myproc()->msgqueue.end == MESSAGE_MAX) {
		myproc()->msgqueue.end = 0;
	}
	release(&myproc()->msgqueue.lock);
	return ret;
}

int sys_message_wait(void) {
	void* data;
	if (argptr(0, (char**)&data, 4 * 1024 * 1024) < 0) {
		return -1;
	}
	acquire(&myproc()->msgqueue.lock);
	while (myproc()->msgqueue.begin == myproc()->msgqueue.end) {
		sleep(&myproc()->msgqueue, &myproc()->msgqueue.lock);
	}
	struct Message* thismsg = &myproc()->msgqueue.queue[myproc()->msgqueue.end];
	int ret = thismsg->pid;
	memmove(data, thismsg->addr, thismsg->size);
	pgfree(thismsg->addr, PGROUNDUP(thismsg->size) / 4096);
	myproc()->msgqueue.end++;
	if (myproc()->msgqueue.end == MESSAGE_MAX) {
		myproc()->msgqueue.end = 0;
	}
	release(&myproc()->msgqueue.lock);
	return ret;
}

int sys_getppid(void) {
	return myproc()->parent->pid;
}

int sys_proc_search(void) {
	char* name;
	if (argstr(0, &name) < 0) {
		return -1;
	}
	acquire(&ptable.lock);
	for (int i = 0; i < NPROC; i++) {
		if (ptable.proc[i].state != UNUSED &&
			strncmp(name, ptable.proc[i].name, sizeof(ptable.proc[i].name)) == 0) {
			release(&ptable.lock);
			return ptable.proc[i].pid;
		}
	}
	release(&ptable.lock);
	return 0;
}

int sys_pty_create(void) {
	return pty_create() + 1;
}

int sys_pty_read_output(void) {
	int ptyid, n;
	char* buf;
	if ((argint(0, &ptyid) < 0) || (argint(2, &n) < 0) || (argptr(1, &buf, n) < 0)) {
		return -1;
	}
	return pty_read_output(ptyid - 1, buf, n);
}

int sys_pty_write_input(void) {
	int ptyid, n;
	char* buf;
	if ((argint(0, &ptyid) < 0) || (argint(2, &n) < 0) || (argptr(1, &buf, n) < 0)) {
		return -1;
	}
	return pty_write_input(ptyid - 1, buf, n);
}

int sys_pty_close(void) {
	int ptyid;
	if (argint(0, &ptyid) < 0) {
		return -1;
	}
	return pty_close(ptyid - 1);
}

int sys_pty_switch(void) {
	int ptyid;
	if (argint(0, &ptyid) < 0) {
		return -1;
	}
	myproc()->pty = ptyid;
	return 0;
}

enum ProcStatus {
	PROC_RUNNING,
	PROC_EXITED,
	PROC_NOT_EXIST,
};

int sys_proc_status(void) {
	int pid;
	int* exit_status;
	if (argint(0, &pid) < 0 || argptr(1, (char**)&exit_status, sizeof(int))) {
		return -1;
	}
	struct proc* p = proc_search_pid(pid);
	if (!p) {
		return PROC_NOT_EXIST;
	}
	acquire(&ptable.lock);
	if (p->state == RUNNING || p->state == RUNNABLE) {
		release(&ptable.lock);
		return PROC_RUNNING;
	} else if (p->state == ZOMBIE) {
		*exit_status = p->exit_status;
		proc_free(p);
		release(&ptable.lock);
		return PROC_EXITED;
	}
	release(&ptable.lock);
	return PROC_NOT_EXIST;
}

int sys_module_load(void) {
	char* name;
	if (argstr(0, &name) < 0) {
		return -1;
	}
	return module_load(name);
}
