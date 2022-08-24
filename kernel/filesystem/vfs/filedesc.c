/*
 * Virtual filesystem file support
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

int vfs_fd_open(struct FileDesc* fd, const char* filename, int mode) {
	memset(fd, 0, sizeof(struct FileDesc));
	struct VfsPath filepath;
	filepath.pathbuf = kalloc();
	filepath.parts = vfs_path_split(filename, filepath.pathbuf);
	if (filename[0] != '/') {
		vfs_get_absolute_path(&filepath);
	}
	struct VfsPath path;
	int fs_id = vfs_path_to_fs(filepath, &path);

	int fblock = vfs_mount_table[fs_id].fs_driver->open(vfs_mount_table[fs_id].private, path);
	if (fblock < 0) {
		if (mode & O_CREATE) {
			// create and retry
			vfs_mount_table[fs_id].fs_driver->create_file(vfs_mount_table[fs_id].private, path);
			fblock = vfs_mount_table[fs_id].fs_driver->open(vfs_mount_table[fs_id].private, path);
		} else {
			kfree(filepath.pathbuf);
			return ERROR_NOT_EXIST;
		}
	}
	fd->block = fblock;
	fd->size
		= vfs_mount_table[fs_id].fs_driver->get_file_size(vfs_mount_table[fs_id].private, path);
	if (mode & O_READ) {
		fd->read = 1;
	}
	if (mode & O_WRITE) {
		fd->path.parts = path.parts;
		fd->path.pathbuf = kalloc();
		memmove(fd->path.pathbuf, path.pathbuf, path.parts * 128);
		fd->write = 1;
		if (mode & O_APPEND) {
			fd->append = 1;
		}
		if (mode & O_TRUNC) {
			fd->size = 0;
		}
	}

	fd->offset = 0;
	fd->fs_id = fs_id;
	fd->used = 1;
	kfree(filepath.pathbuf);
	return 0;
}

int vfs_fd_read(struct FileDesc* fd, void* buf, unsigned int size) {
	if (!fd->used) {
		return ERROR_INVAILD;
	}
	if (!fd->read) {
		return ERROR_INVAILD;
	}
	if (fd->dir) {
		return ERROR_INVAILD;
	}

	int status;
	if (fd->offset >= fd->size) { // EOF
		return 0;
	} else if (fd->offset + size > fd->size) {
		status = fd->size - fd->offset;
	} else {
		status = size;
	}
	int ret = vfs_mount_table[fd->fs_id].fs_driver->read(vfs_mount_table[fd->fs_id].private,
														 fd->block, buf, fd->offset, size);
	if (ret < 0) {
		return ret;
	}
	fd->offset += status;

	return status;
}

int vfs_fd_write(struct FileDesc* fd, const char* buf, unsigned int size) {
	if (!fd->used) {
		return ERROR_INVAILD;
	}
	if (!fd->write) {
		return ERROR_INVAILD;
	}
	if (fd->dir) {
		return ERROR_INVAILD;
	}

	if (fd->append) {
		fd->offset = fd->size;
	}
	int ret = vfs_mount_table[fd->fs_id].fs_driver->write(vfs_mount_table[fd->fs_id].private,
														  fd->block, buf, fd->offset, size);
	if (ret < 0) {
		return ret;
	}
	fd->offset += ret;
	if (fd->offset > fd->size) {
		fd->size += fd->offset - fd->size;
	}
	return ret;
}

int vfs_fd_close(struct FileDesc* fd) {
	if (!fd->used) {
		return ERROR_INVAILD;
	}
	if (fd->dir) {
		return ERROR_INVAILD;
	}

	if (fd->write) {
		vfs_mount_table[fd->fs_id].fs_driver->update_size(vfs_mount_table[fd->fs_id].private,
														  fd->path, fd->size);
		kfree(fd->path.pathbuf);
	}

	fd->used = 0;
	return 0;
}

int vfs_fd_seek(struct FileDesc* fd, unsigned int off, enum FileSeekMode mode) {
	if (!fd->used) {
		return ERROR_INVAILD;
	}
	if (fd->dir) {
		return ERROR_INVAILD;
	}
	if (off >= fd->size) {
		return ERROR_INVAILD;
	}
	switch (mode) {
	case SEEK_SET:
		fd->offset = off;
		break;
	case SEEK_CUR:
		fd->offset += off;
		break;
	case SEEK_END:
		fd->offset = fd->size + off;
		break;
	}
	return fd->offset;
}
