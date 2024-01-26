/*
 * Filesystem system calls
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

#include <common/x86.h>
#include <core/proc.h>
#include <defs.h>
#include <filesystem/initramfs/initramfs.h>
#include <memlayout.h>
#include <param.h>
#include <proc/pty.h>

int sys_dup(void) {
	return 0;
}

int sys_read(void) {
	int n, fd;
	char* p;

	if (argint(0, &fd) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
		return -1;
	if (fd < 3) {
		if (myproc()->pty == 0) {
			return consoleread(p, n);
		} else {
			return pty_read(myproc()->pty - 1, p, n);
		}
	}
	return vfs_fd_read(&myproc()->files[fd], p, n);
}

int sys_write(void) {
	int n, fd;
	char* p;

	if (argint(0, &fd) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
		return -1;
	if (fd < 3) {
		if (myproc()->pty == 0) {
			return consolewrite(p, n);
		} else {
			return pty_write(myproc()->pty - 1, p, n);
		}
	}

	return vfs_fd_write(&myproc()->files[fd], p, n);
}

int sys_close(void) {
	int fd;
	if (argint(0, &fd) < 0) {
		return -1;
	}
	return vfs_fd_close(&myproc()->files[fd]);
}

// Create the path new as a link to the same inode as old.
int sys_link(void) {
	return 0;
}

int sys_unlink(void) {
	char* name;
	if (argstr(0, &name) < 0) {
		return -1;
	}
	return vfs_file_remove(name);
}

int sys_open(void) {
	char* path;
	int omode;

	if (argstr(0, &path) < 0 || argint(1, &omode) < 0)
		return -1;

	for (int i = 3; i < PROC_FILE_MAX; i++) {
		if (myproc()->files[i].used == 0) {
			int ret;
			if ((ret = vfs_fd_open(&myproc()->files[i], path, omode)) < 0) {
				return ret;
			}
			return i;
		}
	}
	return -1;
}

int sys_mkdir(void) {
	char* name;
	if (argstr(0, &name) < 0) {
		return -1;
	}
	return vfs_mkdir(name);
}

int sys_mknod(void) {
	return 0;
}

int sys_exec(void) {
	char *path, *argv[MAXARG];
	int i;
	unsigned int uargv, uarg;

	if (argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0) {
		return -1;
	}
	memset(argv, 0, sizeof(argv));
	for (i = 0;; i++) {
		if (i >= NELEM(argv))
			return -1;
		if (fetchint(uargv + 4 * i, (int*)&uarg) < 0)
			return -1;
		if (uarg == 0) {
			argv[i] = 0;
			break;
		}
		if (fetchstr(uarg, &argv[i]) < 0)
			return -1;
	}
	return exec(path, argv);
}

int sys_pipe(void) {
	return 0;
}

int sys_dir_open(void) {
	char* dirname;
	if (argstr(0, &dirname) < 0) {
		return -1;
	}
	for (int i = 3; i < PROC_FILE_MAX; i++) {
		if (myproc()->files[i].used == 0) {
			if (vfs_dir_open(&myproc()->files[i], dirname) < 0) {
				return -1;
			}
			return i;
		}
	}
	return -1;
}

int sys_dir_read(void) {
	int handle;
	char* buffer;
	if ((argint(0, &handle) < 0) || (argptr(1, &buffer, 256) < 0)) {
		return -1;
	}
	return vfs_dir_read(&myproc()->files[handle], buffer);
}

int sys_dir_close(void) {
	int handle;
	if (argint(0, &handle) < 0) {
		return -1;
	}
	return vfs_dir_close(&myproc()->files[handle]);
}

int sys_file_get_size(void) {
	char* filename;
	if (argstr(0, &filename) < 0) {
		return -1;
	}
	return vfs_file_get_size(filename);
}

int sys_lseek(void) {
	int fd, offset, whence;
	if (argint(0, &fd) < 0 || argint(1, &offset) < 0 || argint(2, &whence) < 0) {
		return -1;
	}
	return vfs_fd_seek(&myproc()->files[fd], offset, whence);
}

int sys_file_get_mode(void) {
	char* filename;
	if (argstr(0, &filename) < 0) {
		return -1;
	}
	return vfs_file_get_mode(filename);
}
