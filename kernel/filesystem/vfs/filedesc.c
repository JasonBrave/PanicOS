/*
 * Virtual filesystem file support
 *
 * This file is part of HoleOS.
 *
 * HoleOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HoleOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HoleOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common/errorcode.h>
#include <defs.h>
#include <filesystem/initramfs/initramfs.h>

#include "vfs.h"

int vfs_fd_open(struct FileDesc* fd, const char* filename, int mode) {
	if (mode != O_READ) {
		return ERROR_INVAILD;
	}
	memset(fd, 0, sizeof(struct FileDesc));

	int blk;
	if ((blk = initramfs_open(filename)) < 0) {
		return blk;
	}
	fd->block = blk;
	fd->offset = 0;
	fd->size = initramfs_file_get_size(filename);
	fd->used = 1;
	fd->read = 1;
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
	status = initramfs_read(fd->block, buf, fd->offset, size);
	if (status > 0) {
		fd->offset += status;
	}
	return status;
}

int vfs_fd_close(struct FileDesc* fd) {
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
