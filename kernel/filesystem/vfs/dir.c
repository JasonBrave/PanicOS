/*
 * Virtual filesystem directory support
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

#include "vfs.h"

int vfs_dir_open(struct FileDesc* fd, const char* dirname) {
	memset(fd, 0, sizeof(struct FileDesc));
	struct VfsPath dirpath;
	dirpath.pathbuf = kalloc();
	dirpath.parts = vfs_path_split(dirname, dirpath.pathbuf);
	if (dirname[0] != '/') {
		vfs_get_absolute_path(&dirpath);
	}
	struct VfsPath path;
	int fs_id = vfs_path_to_fs(dirpath, &path);

	int fblock = vfs_mount_table[fs_id].fs_driver->open(vfs_mount_table[fs_id].private, path);
	if (fblock < 0) {
		kfree(dirpath.pathbuf);
		return fblock;
	}
	fd->block = fblock;
	fd->offset = vfs_mount_table[fs_id].fs_driver->dir_first_file(vfs_mount_table[fs_id].private,
																  fd->block);

	fd->fs_id = fs_id;
	fd->dir = 1;
	fd->read = 1;
	fd->used = 1;
	kfree(dirpath.pathbuf);
	return 0;
}

int vfs_dir_read(struct FileDesc* fd, char* buffer) {
	if (!fd->used) {
		return ERROR_INVAILD;
	}
	if (!fd->read) {
		return ERROR_INVAILD;
	}
	if (!fd->dir) {
		return ERROR_INVAILD;
	}

	int off;
	off = vfs_mount_table[fd->fs_id].fs_driver->dir_read(vfs_mount_table[fd->fs_id].private, buffer,
														 fd->block, fd->offset);

	if (off == 0) {
		return 0; // EOF
	} else if (off < 0) {
		return off; // error
	}
	fd->offset = off;
	return 1;
}

int vfs_dir_close(struct FileDesc* fd) {
	if (!fd->used) {
		return ERROR_INVAILD;
	}
	if (!fd->dir) {
		return ERROR_INVAILD;
	}

	fd->used = 0;
	return 0;
}
